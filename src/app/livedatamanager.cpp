/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "livedatamanager.h"
#include "logging.h"
#include "notificationhelper.h"
#include "pkpassmanager.h"
#include "publictransport.h"
#include "publictransportmatcher.h"
#include "reservationhelper.h"
#include "reservationmanager.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Flight>
#include <KItinerary/LocationUtil>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/JourneyReply>
#include <KPublicTransport/JourneyRequest>
#include <KPublicTransport/Location>
#include <KPublicTransport/Manager>
#include <KPublicTransport/OnboardStatus>
#include <KPublicTransport/TripReply>
#include <KPublicTransport/TripRequest>

#include <KPkPass/Pass>

#include <KLocalizedString>
#include <KNotification>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QList>
#include <QSettings>
#include <QStandardPaths>

#include <cassert>

using namespace KItinerary;

static constexpr const int POLL_COOLDOWN_ON_ERROR = 30; // seconds

LiveDataManager::LiveDataManager(QObject *parent)
    : QObject(parent)
    , m_ptMgr(new KPublicTransport::Manager(this))
    , m_onboardStatus(new KPublicTransport::OnboardStatus(this))
{
    QSettings settings;
    settings.beginGroup(QLatin1StringView("KPublicTransport"));
    m_ptMgr->setAllowInsecureBackends(settings.value(QLatin1StringView("AllowInsecureBackends"), false).toBool());
    m_ptMgr->setDisabledBackends(settings.value(QLatin1StringView("DisabledBackends"), QStringList()).toStringList());
    m_ptMgr->setEnabledBackends(settings.value(QLatin1StringView("EnabledBackends"), QStringList()).toStringList());
    connect(m_ptMgr, &KPublicTransport::Manager::configurationChanged, this, [this]() {
        QSettings settings;
        settings.beginGroup(QLatin1StringView("KPublicTransport"));
        settings.setValue(QLatin1StringView("AllowInsecureBackends"), m_ptMgr->allowInsecureBackends());
        settings.setValue(QLatin1StringView("DisabledBackends"), m_ptMgr->disabledBackends());
        settings.setValue(QLatin1StringView("EnabledBackends"), m_ptMgr->enabledBackends());
    });

    m_pollTimer.setSingleShot(true);
    connect(&m_pollTimer, &QTimer::timeout, this, &LiveDataManager::poll);

    connect(m_onboardStatus, &KPublicTransport::OnboardStatus::journeyChanged, [this]() {
        if (!m_onboardStatus->hasJourney()) {
            return;
        }

        for (const auto &resId : m_reservations) {
            auto res = m_resMgr->reservation(resId);
            if (!hasDeparted(resId, res) || hasArrived(resId, res)) {
                continue;
            }
            const auto journey = m_onboardStatus->journey();
            if (journey.sections().empty()) {
                return;
            }

            applyJourney(resId, journey.sections()[0]);
        }
    });
}

LiveDataManager::~LiveDataManager() = default;

void LiveDataManager::setReservationManager(ReservationManager *resMgr)
{
    assert(m_pkPassMgr);
    m_resMgr = resMgr;
    connect(resMgr, &ReservationManager::batchAdded, this, &LiveDataManager::batchAdded, Qt::QueuedConnection);
    connect(resMgr, &ReservationManager::batchChanged, this, &LiveDataManager::batchChanged);
    connect(resMgr, &ReservationManager::batchContentChanged, this, &LiveDataManager::batchChanged);
    connect(resMgr, &ReservationManager::batchRenamed, this, &LiveDataManager::batchRenamed);
    connect(resMgr, &ReservationManager::batchRemoved, this, &LiveDataManager::batchRemoved);

    const auto resIds = resMgr->batches();
    for (const auto &resId : resIds) {
        if (!isRelevant(resId)) {
            continue;
        }
        m_reservations.push_back(resId);
    }

    m_pollTimer.setInterval(nextPollTime());
}

void LiveDataManager::setPkPassManager(PkPassManager *pkPassMgr)
{
    m_pkPassMgr = pkPassMgr;
    connect(m_pkPassMgr, &PkPassManager::passUpdated, this, &LiveDataManager::pkPassUpdated);
}

void LiveDataManager::setPollingEnabled(bool pollingEnabled)
{
    if (pollingEnabled) {
        m_pollTimer.setInterval(nextPollTime());
        m_pollTimer.start();
    } else {
        m_pollTimer.stop();
    }
}

void LiveDataManager::setShowNotificationsOnLockScreen(bool enabled)
{
    m_showNotificationsOnLockScreen = enabled;
}

KPublicTransport::Stopover LiveDataManager::arrival(const QString &resId) const
{
    return data(resId).arrival();
}

KPublicTransport::Stopover LiveDataManager::departure(const QString &resId) const
{
    return data(resId).departure();
}

KPublicTransport::JourneySection LiveDataManager::journey(const QString &resId) const
{
    return data(resId).journey();
}

void LiveDataManager::setJourney(const QString &resId, const KPublicTransport::JourneySection &journey)
{
    auto &ld = data(resId);
    ld.trip = journey;
    ld.journeyTimestamp = now();
    ld.store(resId);

    Q_EMIT journeyUpdated(resId);
}

void LiveDataManager::checkForUpdates()
{
    pollForUpdates(true);
}

void LiveDataManager::checkForUpdates(const QStringList &batchIds)
{
    clearArrived();
    for (const auto &batchId : batchIds) {
        pollBatchForUpdates(batchId, true);
    }
}

[[nodiscard]] static bool canSelectBackend(const KPublicTransport::Location &loc)
{
    return loc.hasCoordinate() || loc.country().size() == 2;
}

void LiveDataManager::checkReservation(const QVariant &res, const QString &resId)
{
    using namespace KPublicTransport;
    if (hasArrived(resId, res)) {
        return;
    }

    auto jny = data(resId).journey();
    if (jny.mode() == JourneySection::Invalid) {
        jny.setMode(JourneySection::PublicTransport);
        jny.setFrom(PublicTransport::locationFromPlace(LocationUtil::departureLocation(res), res));
        jny.setTo(PublicTransport::locationFromPlace(LocationUtil::arrivalLocation(res), res));
        jny.setScheduledDepartureTime(SortUtil::startDateTime(res));
        jny.setRoute(PublicTransport::routeForReservation(res));
    }

    TripRequest req(jny);
    req.setDownloadAssets(true);
    PublicTransport::selectBackends(req, m_ptMgr, res);
    if (canSelectBackend(jny.from()) || canSelectBackend(jny.to()) || !req.backendIds().isEmpty() || jny.hasIdentifiers()) {
        auto reply = m_ptMgr->queryTrip(req);
        connect(reply, &Reply::finished, this, [this, resId, reply]() {
            reply->deleteLater();
            if (reply->error() == Reply::NoError) {
                auto jny = reply->journeySection();
                if (jny.mode() == JourneySection::PublicTransport) {
                    applyJourney(resId, jny);
                    return;
                }
            }
            // record this is a failed lookup so we don't try again
            tripQueryFailed(resId);
        });
        m_lastPollAttempt.insert(resId, now());
    }
}

void LiveDataManager::tripQueryFailed(const QString &resId)
{
    data(resId).journeyTimestamp = now();
}

static KPublicTransport::Stopover applyLayoutData(const KPublicTransport::Stopover &stop, const KPublicTransport::Stopover &layout)
{
    auto res = stop;
    if (stop.vehicleLayout().isEmpty()) {
        res.setVehicleLayout(layout.vehicleLayout());
    }
    if (stop.platformLayout().isEmpty()) {
        res.setPlatformLayout(layout.platformLayout());
    }
    return res;
}

static void applyMissingStopoverData(KPublicTransport::Stopover &stop, const KPublicTransport::Stopover &oldStop)
{
    if (!stop.stopPoint().hasCoordinate()) { // DB's new API doesn't have coordinates in stopover queries...
        auto s = stop.stopPoint();
        s.setCoordinate(oldStop.stopPoint().latitude(), oldStop.stopPoint().longitude());
        stop.setStopPoint(s);
    }
    if (stop.notes().empty()) {
        stop.setNotes(oldStop.notes());
    }
    if (stop.loadInformation().empty()) {
        stop.setLoadInformation(std::vector<KPublicTransport::LoadInfo>(oldStop.loadInformation()));
    }
}

static void applyMissingJourneyData(KPublicTransport::JourneySection &journey, const KPublicTransport::JourneySection &oldJny)
{
    if (journey.intermediateStops().size() != oldJny.intermediateStops().size()) {
        return;
    }

    auto stops = journey.takeIntermediateStops();
    for (std::size_t i = 0; i < stops.size(); ++i) {
        if (!KPublicTransport::Stopover::isSame(stops[i], oldJny.intermediateStops()[i])) {
            journey.setIntermediateStops(std::move(stops));
            return;
        }
        applyMissingStopoverData(stops[i], oldJny.intermediateStops()[i]);
    }
    journey.setIntermediateStops(std::move(stops));

    if (!KPublicTransport::Stopover::isSame(journey.departure(), oldJny.departure())
        || !KPublicTransport::Stopover::isSame(journey.arrival(), oldJny.arrival())) {
        return;
    }
    auto s = journey.departure();
    applyMissingStopoverData(s, oldJny.departure());
    journey.setDeparture(s);
    s = journey.arrival();
    applyMissingStopoverData(s, oldJny.arrival());
    journey.setArrival(s);

    if (journey.path().isEmpty()) {
        journey.setPath(oldJny.path());
    }
    if (journey.notes().empty()) {
        journey.setNotes(oldJny.notes());
    }
}

void LiveDataManager::applyJourney(const QString &resId, const KPublicTransport::JourneySection &journey)
{
    // align with sub/super-journey with what we already have
    const auto subjny = PublicTransportMatcher::subJourneyForReservation(m_resMgr->reservation(resId), journey);
    if (subjny.mode() == KPublicTransport::JourneySection::Invalid) {
        qCDebug(Log) << "Failed to find sub journey";
        return;
    }

    auto &ld = data(resId);
    const auto oldJny = ld.trip;
    ld.trip = subjny;

    // retain already existing vehicle/platform layout data if we are still departing/arriving in the same place
    if (PublicTransport::isSameStopoverForLayout(oldJny.departure(), ld.trip.departure())) {
        ld.trip.setDeparture(applyLayoutData(ld.trip.departure(), oldJny.departure()));
    }
    if (PublicTransport::isSameStopoverForLayout(oldJny.arrival(), ld.trip.arrival())) {
        ld.trip.setArrival(applyLayoutData(ld.trip.arrival(), oldJny.arrival()));
    }
    applyMissingJourneyData(ld.trip, oldJny);

    ld.trip.applyMetaData(true); // download logo assets if needed
    ld.journeyTimestamp = now();
    ld.store(resId);

    // update reservation with live data
    std::vector<ReservationManager::ReservationChange> resUpdates;
    for (const auto &id : m_resMgr->reservationsForBatch(resId)) {
        const auto r = m_resMgr->reservation(id);
        const auto newRes = PublicTransport::mergeJourney(r, ld.journey());
        if (!ReservationHelper::equals(r, newRes) && r.typeId() == newRes.typeId()) {
            resUpdates.emplace_back(id, newRes);
        }
    }
    qCDebug(Log) << "submitting" << resUpdates.size() << "reservation changes";
    m_resMgr->updateBatch(resUpdates);

    // emit update signals
    Q_EMIT journeyUpdated(resId);

    // check if we need to notify
    if (NotificationHelper::shouldNotify(oldJny.departure(), ld.departure(), LiveData::Departure)
        || NotificationHelper::shouldNotify(oldJny.arrival(), ld.arrival(), LiveData::Arrival)) {
        showNotification(resId, ld);
    }
}

void LiveDataManager::showNotification(const QString &resId, const LiveData &ld)
{
    // check if we still have an active notification, if so, update that one
    const auto it = m_notifications.constFind(resId);
    if (it == m_notifications.cend() || !it.value()) {
        auto n = new KNotification(QStringLiteral("disruption"));
        fillNotification(n, ld);
        m_notifications.insert(resId, n);
        n->sendEvent();
    } else {
        fillNotification(it.value(), ld);
    }
}

void LiveDataManager::fillNotification(KNotification *n, const LiveData &ld) const
{
    n->setTitle(NotificationHelper::title(ld));
    n->setText(NotificationHelper::message(ld));
    n->setIconName(QLatin1StringView("clock"));
    if (m_showNotificationsOnLockScreen) {
        n->setHint(QStringLiteral("x-kde-visibility"), QStringLiteral("public"));
    }
}

void LiveDataManager::showNotification(const QString &resId)
{
    // this is only meant for testing!
    showNotification(resId, data(resId));
}

void LiveDataManager::cancelNotification(const QString &resId)
{
    const auto nIt = m_notifications.find(resId);
    if (nIt != m_notifications.end()) {
        if (nIt.value()) {
            nIt.value()->close();
        }
        m_notifications.erase(nIt);
    }
}

QDateTime LiveDataManager::departureTime(const QString &resId, const QVariant &res) const
{
    if (JsonLd::isA<TrainReservation>(res) || JsonLd::isA<BusReservation>(res)) {
        const auto &dep = departure(resId);
        if (dep.hasExpectedDepartureTime()) {
            return dep.expectedDepartureTime();
        }
    }

    return SortUtil::startDateTime(res);
}

QDateTime LiveDataManager::arrivalTime(const QString &resId, const QVariant &res) const
{
    if (JsonLd::isA<TrainReservation>(res) || JsonLd::isA<BusReservation>(res)) {
        const auto &arr = arrival(resId);
        if (arr.hasExpectedArrivalTime()) {
            return arr.expectedArrivalTime();
        }
    }

    return SortUtil::endDateTime(res);
}

bool LiveDataManager::hasDeparted(const QString &resId, const QVariant &res) const
{
    return departureTime(resId, res) < now();
}

bool LiveDataManager::hasArrived(const QString &resId, const QVariant &res) const
{
    const auto n = now();
    // avoid loading live data for everything on startup
    if (SortUtil::endDateTime(res).addDays(1) < n) {
        return true;
    }
    return arrivalTime(resId, res) < now();
}

LiveData &LiveDataManager::data(const QString &resId) const
{
    auto it = m_data.find(resId);
    if (it != m_data.end()) {
        return it.value();
    }

    it = m_data.insert(resId, LiveData::load(resId));
    return it.value();
}

void LiveDataManager::importData(const QString &resId, LiveData &&data)
{
    // we don't need to store data, Importer already does that
    m_data[resId] = std::move(data);
    Q_EMIT journeyUpdated(resId);
}

bool LiveDataManager::isRelevant(const QString &resId) const
{
    const auto res = m_resMgr->reservation(resId);
    // we only care about transit reservations
    if (!JsonLd::canConvert<Reservation>(res) || !LocationUtil::isLocationChange(res) || ReservationHelper::isCancelled(res)) {
        return false;
    }
    // we don't care about past events
    if (hasArrived(resId, res)) {
        return false;
    }

    // things handled by KPublicTransport
    if (JsonLd::isA<TrainReservation>(res) || JsonLd::isA<BusReservation>(res)) {
        return true;
    }

    // things with an updatable pkpass
    const auto passId = PkPassManager::passId(res);
    if (passId.isEmpty()) {
        return false;
    }
    const auto pass = m_pkPassMgr->pass(passId);
    return PkPassManager::canUpdate(pass);
}

void LiveDataManager::batchAdded(const QString &resId)
{
    if (!isRelevant(resId)) {
        return;
    }

    m_reservations.push_back(resId);
    m_pollTimer.setInterval(nextPollTime());
}

void LiveDataManager::batchChanged(const QString &resId)
{
    const auto it = std::find(m_reservations.begin(), m_reservations.end(), resId);
    const auto relevant = isRelevant(resId);

    if (it == m_reservations.end() && relevant) {
        m_reservations.push_back(resId);
    } else if (it != m_reservations.end() && !relevant) {
        m_reservations.erase(it);
    }

    // check if existing updates still apply, and remove them otherwise!
    const auto res = m_resMgr->reservation(resId);
    const auto dataIt = m_data.find(resId);
    if (dataIt != m_data.end()) {
        if ((*dataIt).journeyTimestamp.isValid() && !PublicTransportMatcher::isJourneyForReservation(res, (*dataIt).journey())) {
            (*dataIt).trip = {};
            (*dataIt).journeyTimestamp = {};
            (*dataIt).store(resId);
            Q_EMIT journeyUpdated(resId);
        }
    }

    m_pollTimer.setInterval(nextPollTime());
}

void LiveDataManager::batchRenamed(const QString &oldBatchId, const QString &newBatchId)
{
    const auto it = std::find(m_reservations.begin(), m_reservations.end(), oldBatchId);
    if (it != m_reservations.end()) {
        *it = newBatchId;
    }
}

void LiveDataManager::batchRemoved(const QString &resId)
{
    const auto it = std::find(m_reservations.begin(), m_reservations.end(), resId);
    if (it != m_reservations.end()) {
        m_reservations.erase(it);
    }

    cancelNotification(resId);
    LiveData::remove(resId);
    m_data.remove(resId);
    m_lastPollAttempt.remove(resId);
}

void LiveDataManager::poll()
{
    qCDebug(Log);
    pollForUpdates(false);

    m_pollTimer.setInterval(std::max(nextPollTime(), 60 * 1000)); // we pool everything that happens within a minute here
    m_pollTimer.start();
}

void LiveDataManager::pollForUpdates(bool force)
{
    clearArrived();

    for (const auto &batchId : m_reservations) {
        pollBatchForUpdates(batchId, force);
    }
}

void LiveDataManager::pollBatchForUpdates(const QString &batchId, bool force)
{
    if (!force && nextPollTimeForReservation(batchId) > 60 * 1000) {
        // data is still "fresh" according to the poll policy
        return;
    }

    const auto res = m_resMgr->reservation(batchId);
    if (hasArrived(batchId, res)) {
        return;
    }

    if (JsonLd::isA<TrainReservation>(res) || JsonLd::isA<BusReservation>(res)) {
        checkReservation(res, batchId);
    }

    // check for pkpass updates, for each element in this batch
    const auto resIds = m_resMgr->reservationsForBatch(batchId);
    for (const auto &resId : resIds) {
        const auto res = m_resMgr->reservation(resId);
        const auto passId = PkPassManager::passId(res);
        if (!passId.isEmpty()) {
            m_lastPollAttempt.insert(batchId, now());
            m_pkPassMgr->updatePass(passId);
        }
    }
}

void LiveDataManager::clearArrived()
{
    for (auto it = m_reservations.begin(); it != m_reservations.end();) {
        const auto batchId = *it;
        const auto res = m_resMgr->reservation(*it);

        // clean up obsolete stuff
        if (hasArrived(*it, res)) {
            cancelNotification(*it);
            it = m_reservations.erase(it);
            m_lastPollAttempt.remove(batchId);
        } else {
            ++it;
        }
    }
}

int LiveDataManager::nextPollTime() const
{
    int t = std::numeric_limits<int>::max();
    for (const auto &resId : m_reservations) {
        t = std::min(t, nextPollTimeForReservation(resId));
    }
    qCDebug(Log) << "next auto-update in" << (t / 1000) << "secs";
    return t;
}

static constexpr const int MAX_POLL_INTERVAL = 7 * 24 * 3600;
struct {
    int distance; // secs
    int pollInterval; // secs
} static const pollIntervalTable[] = {
    {3600, 5 * 60}, // for <1h we poll every 5 minutes
    {4 * 3600, 15 * 60}, // for <4h we poll every 15 minutes
    {24 * 3600, 3600}, // for <1d we poll once per hour
    {4 * 24 * 3600, 24 * 3600}, // for <4d we poll once per day
    {std::numeric_limits<int>::max(), MAX_POLL_INTERVAL}, // anything before we should at least do one poll to get full details right away
};

int LiveDataManager::nextPollTimeForReservation(const QString &resId) const
{
    const auto res = m_resMgr->reservation(resId);

    const auto now = this->now();
    auto dist = now.secsTo(departureTime(resId, res));
    if (dist < 0) {
        dist = now.secsTo(arrivalTime(resId, res));
    }
    if (dist < 0) {
        return std::numeric_limits<int>::max();
    }

    const auto it = std::lower_bound(std::begin(pollIntervalTable), std::end(pollIntervalTable), dist, [](const auto &lhs, const auto rhs) {
        return lhs.distance < rhs;
    });
    if (it == std::end(pollIntervalTable)) {
        return std::numeric_limits<int>::max();
    }

    // check last poll time for this reservation
    const auto &ld = data(resId);
    const auto lastRelevantPoll = lastPollTime(resId, res);
    const int lastPollDist = (!lastRelevantPoll.isValid() || lastRelevantPoll > now) ? MAX_POLL_INTERVAL // no poll yet == long time ago
                                                                                     : lastRelevantPoll.secsTo(now);
    return std::max((it->pollInterval - lastPollDist) * 1000, pollCooldown(resId)); // we need msecs
}

QDateTime LiveDataManager::lastPollTime(const QString &batchId, const QVariant &res) const
{
    auto dt = data(batchId).journeyTimestamp;
    if (dt.isValid()) {
        return dt;
    }

    // check for pkpass updates
    const auto resIds = m_resMgr->reservationsForBatch(batchId);
    for (const auto &resId : resIds) {
        const auto res = m_resMgr->reservation(resId);
        const auto passId = m_pkPassMgr->passId(res);
        if (!passId.isEmpty()) {
            dt = m_pkPassMgr->updateTime(passId);
        }
        if (dt.isValid()) {
            return dt;
        }
    }

    return dt;
}

int LiveDataManager::pollCooldown(const QString &resId) const
{
    const auto lastPollTime = m_lastPollAttempt.value(resId);
    if (!lastPollTime.isValid()) {
        return 0;
    }
    return std::clamp<int>(POLL_COOLDOWN_ON_ERROR - lastPollTime.secsTo(now()), 0, POLL_COOLDOWN_ON_ERROR) * 1000;
}

void LiveDataManager::pkPassUpdated(const QString &passId, const QStringList &changes)
{
    if (changes.isEmpty()) {
        return;
    }

    QVariant passRes;

    // Find relevant reservation for the given passId.
    for (const QString &resId : std::as_const(m_reservations)) {
        const auto res = m_resMgr->reservation(resId);
        const auto resPassId = PkPassManager::passId(res);
        if (resPassId == passId) {
            passRes = res;
            break;
        }
    }

    QString text = changes.join(QLatin1Char('\n'));

    if (JsonLd::isA<FlightReservation>(passRes)) {
        const auto flight = passRes.value<FlightReservation>().reservationFor().value<Flight>();

        text.prepend(QLatin1Char('\n'));
        text.prepend(i18n("Flight %1 to %2:",
                          // TODO add formatter util for this.
                          flight.airline().iataCode() + QLatin1Char(' ') + flight.flightNumber(),
                          LocationUtil::name(LocationUtil::arrivalLocation(passRes))));
    }

    KNotification::event(KNotification::Notification, i18n("Itinerary change"), text, QLatin1StringView("clock"));
}

KPublicTransport::Manager *LiveDataManager::publicTransportManager() const
{
    return m_ptMgr;
}

QDateTime LiveDataManager::now() const
{
    if (Q_UNLIKELY(m_unitTestTime.isValid())) {
        return m_unitTestTime;
    }
    return QDateTime::currentDateTime();
}

#include "moc_livedatamanager.cpp"
