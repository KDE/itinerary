/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "livedatamanager.h"
#include "logging.h"
#include "notificationhelper.h"
#include "pkpassmanager.h"
#include "reservationhelper.h"
#include "reservationmanager.h"
#include "publictransport.h"

#include <KItinerary/BusTrip>
#include <KItinerary/LocationUtil>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/JourneyReply>
#include <KPublicTransport/JourneyRequest>
#include <KPublicTransport/Location>
#include <KPublicTransport/Manager>
#include <KPublicTransport/StopoverReply>
#include <KPublicTransport/StopoverRequest>

#include <KLocalizedString>
#include <KNotification>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QStandardPaths>
#include <QVector>

using namespace KItinerary;

LiveDataManager::LiveDataManager(QObject *parent)
    : QObject(parent)
    , m_ptMgr(new KPublicTransport::Manager(this))
{
    QSettings settings;
    settings.beginGroup(QLatin1String("KPublicTransport"));
    m_ptMgr->setAllowInsecureBackends(settings.value(QLatin1String("AllowInsecureBackends"), false).toBool());
    m_ptMgr->setDisabledBackends(settings.value(QLatin1String("DisabledBackends"), QStringList()).toStringList());
    m_ptMgr->setEnabledBackends(settings.value(QLatin1String("EnabledBackends"), QStringList()).toStringList());
    connect(m_ptMgr, &KPublicTransport::Manager::configurationChanged, this, [this]() {
        QSettings settings;
        settings.beginGroup(QLatin1String("KPublicTransport"));
        settings.setValue(QLatin1String("AllowInsecureBackends"), m_ptMgr->allowInsecureBackends());
        settings.setValue(QLatin1String("DisabledBackends"), m_ptMgr->disabledBackends());
        settings.setValue(QLatin1String("EnabledBackends"), m_ptMgr->enabledBackends());
    });

    m_pollTimer.setSingleShot(true);
    connect(&m_pollTimer, &QTimer::timeout, this, &LiveDataManager::poll);
}

LiveDataManager::~LiveDataManager() = default;

void LiveDataManager::setReservationManager(ReservationManager *resMgr)
{
    m_resMgr = resMgr;
    connect(resMgr, &ReservationManager::batchAdded, this, &LiveDataManager::batchAdded);
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
    return data(resId).arrival;
}

KPublicTransport::Stopover LiveDataManager::departure(const QString &resId) const
{
    return data(resId).departure;
}

KPublicTransport::JourneySection LiveDataManager::journey(const QString &resId) const
{
    return data(resId).journey;
}

void LiveDataManager::setJourney(const QString &resId, const KPublicTransport::JourneySection &journey)
{
    auto &ld = data(resId);
    ld.journey = journey;
    ld.journeyTimestamp = now();
    ld.departure = journey.departure();
    ld.departureTimestamp = now();
    ld.arrival = journey.arrival();
    ld.arrivalTimestamp = now();
    ld.store(resId, LiveData::AllTypes);

    Q_EMIT journeyUpdated(resId);
    Q_EMIT departureUpdated(resId);
    Q_EMIT arrivalUpdated(resId);
}

void LiveDataManager::checkForUpdates()
{
    pollForUpdates(true);
}

static bool isSameLine(const KPublicTransport::Line &lhs, const QString &trainName, const QString &trainNumber)
{
    KPublicTransport::Line rhs;
    rhs.setModeString(trainName);
    rhs.setName(trainNumber);
    return KPublicTransport::Line::isSame(lhs, rhs);
}

static bool isDepartureForReservation(const QVariant &res, const KPublicTransport::Stopover &dep)
{
    const auto lineData = ReservationHelper::lineNameAndNumber(res);
    return PublicTransport::isSameMode(res, dep.route().line().mode())
        && SortUtil::startDateTime(res) == dep.scheduledDepartureTime()
        && isSameLine(dep.route().line(), lineData.first, lineData.second);
}

static bool isArrivalForReservation(const QVariant &res, const KPublicTransport::Stopover &arr)
{
    const auto lineData = ReservationHelper::lineNameAndNumber(res);
    return PublicTransport::isSameMode(res, arr.route().line().mode())
        && SortUtil::endDateTime(res) == arr.scheduledArrivalTime()
        && isSameLine(arr.route().line(), lineData.first, lineData.second);
}

static bool isJourneyForReservation(const QVariant &res, const KPublicTransport::JourneySection &journey)
{
    const auto lineData = ReservationHelper::lineNameAndNumber(res);
    return PublicTransport::isSameMode(res, journey.route().line().mode())
        && SortUtil::startDateTime(res) == journey.scheduledDepartureTime()
        && isSameLine(journey.route().line(), lineData.first, lineData.second);
}

void LiveDataManager::checkReservation(const QVariant &res, const QString& resId)
{
    using namespace KPublicTransport;
    const auto arrived = hasArrived(resId, res);

    // load full journey if we don't have one yet
    if (!arrived && data(resId).journey.mode() == JourneySection::Invalid) {
        const auto from = PublicTransport::locationFromPlace(LocationUtil::departureLocation(res), res);
        const auto to = PublicTransport::locationFromPlace(LocationUtil::arrivalLocation(res), res);
        JourneyRequest req(from, to);
        // start searching slightly earlier, so leading walking section because our coordinates
        // aren't exactly at the right spot wont make the routing service consider the train we
        // are looking for as impossible to reach on time
        req.setDateTime(SortUtil::startDateTime(res).addSecs(-600));
        req.setDateTimeMode(JourneyRequest::Departure);
        req.setIncludeIntermediateStops(true);
        req.setIncludePaths(true);
        req.setModes(JourneySection::PublicTransport);
        PublicTransport::selectBackends(req, m_ptMgr, res);
        auto reply = m_ptMgr->queryJourney(req);
        connect(reply, &Reply::finished, this, [this, resId, reply]() { journeyQueryFinished(reply, resId); });
        return;
    }

    if (!hasDeparted(resId, res)) {
        StopoverRequest req(PublicTransport::locationFromPlace(LocationUtil::departureLocation(res), res));
        req.setMode(StopoverRequest::QueryDeparture);
        req.setDateTime(SortUtil::startDateTime(res));
        PublicTransport::selectBackends(req, m_ptMgr, res);
        auto reply = m_ptMgr->queryStopover(req);
        connect(reply, &Reply::finished, this, [this, resId, reply]() { stopoverQueryFinished(reply, LiveData::Departure, resId); });
    }

    if (!arrived) {
        StopoverRequest req(PublicTransport::locationFromPlace(LocationUtil::arrivalLocation(res), res));
        req.setMode(StopoverRequest::QueryArrival);
        req.setDateTime(SortUtil::endDateTime(res));
        PublicTransport::selectBackends(req, m_ptMgr, res);
        auto reply = m_ptMgr->queryStopover(req);
        connect(reply, &Reply::finished, this, [this, resId, reply]() { stopoverQueryFinished(reply, LiveData::Arrival, resId); });
    }
}

void LiveDataManager::stopoverQueryFinished(KPublicTransport::StopoverReply* reply, LiveData::Type type, const QString& resId)
{
    reply->deleteLater();
    if (reply->error() != KPublicTransport::Reply::NoError) {
        qCDebug(Log) << reply->error() << reply->errorString();
        return;
    }
    stopoverQueryFinished(reply->takeResult(), type, resId);
}

void LiveDataManager::stopoverQueryFinished(std::vector<KPublicTransport::Stopover> &&result, LiveData::Type type, const QString& resId)
{
    const auto res = m_resMgr->reservation(resId);
    for (const auto &stop : result) {
        qCDebug(Log) << "Got stopover information:" << stop.route().line().name() << stop.scheduledDepartureTime();
        if (type == LiveData::Arrival ? isArrivalForReservation(res, stop) : isDepartureForReservation(res, stop)) {
            qCDebug(Log) << "Found stopover information:" << stop.route().line().name() << stop.expectedPlatform() << stop.expectedDepartureTime();
            updateStopoverData(stop, type, resId, res);
            return;
        }
    }

    // record this is a failed lookup so we don't try again
    data(resId).setTimestamp(type, now());
}

void LiveDataManager::journeyQueryFinished(KPublicTransport::JourneyReply *reply, const QString &resId)
{
    reply->deleteLater();
    if (reply->error() != KPublicTransport::Reply::NoError) {
        qCDebug(Log) << reply->error() << reply->errorString();
        return;
    }

    using namespace KPublicTransport;
    const auto res = m_resMgr->reservation(resId);
    for (const auto &journey : reply->result()) {
        if (std::count_if(journey.sections().begin(), journey.sections().end(), [](const auto &sec) { return sec.mode() == JourneySection::PublicTransport; }) != 1) {
            continue;
        }
        const auto it = std::find_if(journey.sections().begin(), journey.sections().end(), [](const auto &sec) { return sec.mode() == JourneySection::PublicTransport; });
        assert(it != journey.sections().end());
        qCDebug(Log) << "Got journey information:" << (*it).route().line().name() << (*it).scheduledDepartureTime();
        if (isJourneyForReservation(res, (*it))) {
            qCDebug(Log) << "Found journey information:" << (*it).route().line().name() << (*it).expectedDeparturePlatform() << (*it).expectedDepartureTime();
            updateJourneyData((*it), resId, res);
            return;
        }
    }

    // record this is a failed lookup so we don't try again
    data(resId).setTimestamp(LiveData::Arrival, now());
    data(resId).setTimestamp(LiveData::Departure, now());
}

void LiveDataManager::updateStopoverData(const KPublicTransport::Stopover &stop, LiveData::Type type, const QString &resId, const QVariant &res)
{
    auto &ld = data(resId);
    const auto oldStop = ld.stopover(type);
    auto newStop = stop;
    newStop.applyMetaData(true); // download logo assets if needed
    ld.setStopover(type, newStop);
    ld.setTimestamp(type, now());
    ld.store(resId);

    // update reservation with live data
    const auto newRes = type == LiveData::Arrival ? PublicTransport::mergeArrival(res, newStop) : PublicTransport::mergeDeparture(res, newStop);
    if (!ReservationHelper::equals(res, newRes)) {
        m_resMgr->updateReservation(resId, newRes);
    }

    // emit update signals
    Q_EMIT type == LiveData::Arrival ? arrivalUpdated(resId) : departureUpdated(resId);

    // check if we need to notify
    if (NotificationHelper::shouldNotify(oldStop, newStop, type)) {
        showNotification(resId, ld);
    }
}

void LiveDataManager::updateJourneyData(const KPublicTransport::JourneySection &journey, const QString &resId, const QVariant &res)
{
    auto &ld = data(resId);
    const auto oldDep = ld.stopover(LiveData::Departure);
    const auto oldArr = ld.stopover(LiveData::Arrival);
    ld.journey = journey;
    ld.journey.applyMetaData(true); // download logo assets if needed
    ld.journeyTimestamp = now();
    ld.departure = journey.departure();
    ld.departureTimestamp = now();
    ld.arrival = journey.arrival();
    ld.arrivalTimestamp = now();
    ld.store(resId, LiveData::AllTypes);

    // update reservation with live data
    const auto newRes = PublicTransport::mergeJourney(res, journey);
    if (!ReservationHelper::equals(res, newRes)) {
        m_resMgr->updateReservation(resId, newRes);
    }

    // emit update signals
    Q_EMIT journeyUpdated(resId);
    Q_EMIT departureUpdated(resId);
    Q_EMIT arrivalUpdated(resId);

    // check if we need to notify
    if (NotificationHelper::shouldNotify(oldDep, journey.departure(), LiveData::Departure) ||
        NotificationHelper::shouldNotify(oldArr, journey.arrival(), LiveData::Arrival)) {
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
        it.value()->update();
    }
}

void LiveDataManager::fillNotification(KNotification* n, const LiveData& ld) const
{
    n->setTitle(NotificationHelper::title(ld));
    n->setText(NotificationHelper::message(ld));
    n->setIconName(QLatin1String("clock"));
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

LiveData& LiveDataManager::data(const QString &resId) const
{
    auto it = m_data.find(resId);
    if (it != m_data.end()) {
        return it.value();
    }

    it = m_data.insert(resId, LiveData::load(resId));
    return it.value();
}

void LiveDataManager::importData(const QString& resId, LiveData &&data)
{
    // we don't need to store data, Importer already does that
    m_data[resId] = std::move(data);
    Q_EMIT journeyUpdated(resId);
    Q_EMIT departureUpdated(resId);
    Q_EMIT arrivalUpdated(resId);
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

    // TODO: we could discard non-train trips without a pkpass in their batch here?

    return true;
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
        if ((*dataIt).departureTimestamp.isValid() && !isDepartureForReservation(res, (*dataIt).departure)) {
            (*dataIt).departure = {};
            (*dataIt).departureTimestamp = {};
            (*dataIt).store(resId, LiveData::Departure);
            Q_EMIT departureUpdated(resId);
        }
        if ((*dataIt).arrivalTimestamp.isValid() && !isArrivalForReservation(res, (*dataIt).arrival)) {
            (*dataIt).arrival = {};
            (*dataIt).arrivalTimestamp = {};
            (*dataIt).store(resId, LiveData::Arrival);
            Q_EMIT arrivalUpdated(resId);
        }

        if ((*dataIt).journeyTimestamp.isValid() && !isJourneyForReservation(res, (*dataIt).journey)) {
            (*dataIt).journey = {};
            (*dataIt).journeyTimestamp = {};
            (*dataIt).store(resId, LiveData::Journey);
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
    for (auto it = m_reservations.begin(); it != m_reservations.end();) {
        const auto batchId = *it;
        const auto res = m_resMgr->reservation(*it);

        // clean up obsolete stuff
        if (hasArrived(*it, res)) {
            cancelNotification(*it);
            it = m_reservations.erase(it);
            continue;
        }
        ++it;

        if (!force && nextPollTimeForReservation(batchId) > 60 * 1000) {
            // data is still "fresh" according to the poll policy
            continue;
        }

        if (JsonLd::isA<TrainReservation>(res) || JsonLd::isA<BusReservation>(res)) {
            checkReservation(res, batchId);
        }

        // check for pkpass updates, for each element in this batch
        const auto resIds = m_resMgr->reservationsForBatch(batchId);
        for (const auto &resId : resIds) {
            const auto res = m_resMgr->reservation(resId);
            const auto passId = m_pkPassMgr->passId(res);
            if (!passId.isEmpty()) {
                m_pkPassMgr->updatePass(passId);
            }
        }
    }
}

int LiveDataManager::nextPollTime() const
{
    int t = std::numeric_limits<int>::max();
    for (const auto &resId : m_reservations) {
        t = std::min(t, nextPollTimeForReservation(resId));
    }
    qCDebug(Log) << "next auto-update in" << (t/1000) << "secs";
    return t;
}

static constexpr const int MAX_POLL_INTERVAL = 7 * 24 * 3600;
struct {
    int distance; // secs
    int pollInterval; // secs
} static const pollIntervalTable[] = {
    { 3600, 5*60 }, // for <1h we poll every 5 minutes
    { 4 * 3600, 15 * 60 }, // for <4h we poll every 15 minutes
    { 24 * 3600, 3600 }, // for <1d we poll once per hour
    { 4 * 24 * 3600, 24 * 3600 }, // for <4d we poll once per day
    { 60 * 24 * 3600, MAX_POLL_INTERVAL }, // anything before we should at least do one poll to get full details right away
};

int LiveDataManager::nextPollTimeForReservation(const QString& resId) const
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
    const auto lastArrivalPoll = ld.arrivalTimestamp;
    const auto lastDeparturePoll = lastDeparturePollTime(resId, res);
    auto lastRelevantPoll = lastArrivalPoll;
    // ignore departure if we have already departed
    if (!hasDeparted(resId, res) && lastDeparturePoll.isValid()) {
        if (!lastArrivalPoll.isValid() || lastArrivalPoll > lastDeparturePoll) {
            lastRelevantPoll = lastDeparturePoll;
        }
    }
    const int lastPollDist = !lastRelevantPoll.isValid()
        ? MAX_POLL_INTERVAL // no poll yet == long time ago
        : lastRelevantPoll.secsTo(now);
    return std::max((it->pollInterval - lastPollDist) * 1000, 0); // we need msecs
}

QDateTime LiveDataManager::lastDeparturePollTime(const QString &batchId, const QVariant &res) const
{
    auto dt = data(batchId).departureTimestamp;
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

void LiveDataManager::pkPassUpdated(const QString &passId, const QStringList &changes)
{
    Q_UNUSED(passId)
    // ### to provide more context, we need to have a passId -> batchId map here eventually

    if (!changes.isEmpty()) {
        KNotification::event(KNotification::Notification, i18n("Itinerary change"), changes.join(QLatin1Char('\n')), QLatin1String("clock"));
    }
}

KPublicTransport::Manager* LiveDataManager::publicTransportManager() const
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
