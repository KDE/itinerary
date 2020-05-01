/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

#include <KPublicTransport/Location>
#include <KPublicTransport/Manager>
#include <KPublicTransport/StopoverReply>
#include <KPublicTransport/StopoverRequest>

#include <KNotifications/KNotification>

#include <KLocalizedString>

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

KPublicTransport::Stopover LiveDataManager::arrival(const QString &resId) const
{
    return data(resId).arrival;
}

KPublicTransport::Stopover LiveDataManager::departure(const QString &resId) const
{
    return data(resId).departure;
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

    emit departureUpdated(resId);
    emit arrivalUpdated(resId);
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

void LiveDataManager::checkReservation(const QVariant &res, const QString& resId)
{
    using namespace KPublicTransport;

    if (!hasDeparted(resId, res)) {
        StopoverRequest req(PublicTransport::locationFromPlace(LocationUtil::departureLocation(res), res));
        req.setMode(StopoverRequest::QueryDeparture);
        req.setDateTime(SortUtil::startDateTime(res));
        auto reply = m_ptMgr->queryStopover(req);
        connect(reply, &Reply::finished, this, [this, resId, reply]() { stopoverQueryFinished(reply, LiveData::Departure, resId); });
    }

    if (!hasArrived(resId, res)) {
        StopoverRequest req(PublicTransport::locationFromPlace(LocationUtil::arrivalLocation(res), res));
        req.setMode(StopoverRequest::QueryArrival);
        req.setDateTime(SortUtil::endDateTime(res));
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

void LiveDataManager::updateStopoverData(const KPublicTransport::Stopover &stop, LiveData::Type type, const QString &resId, const QVariant &res)
{
    auto &ld = data(resId);
    const auto oldStop = ld.stopover(type);
    ld.setStopover(type, stop);
    ld.setTimestamp(type, now());
    ld.store(resId);

    // TODO update reservation with live data
    // TODO emit update signals
    type == LiveData::Arrival ? updateArrivalData(stop, resId) : updateDepartureData(stop, resId); // ### temporary

    // check if we need to notify
    if (!NotificationHelper::shouldNotify(oldStop, stop, type)) {
        return;
    }

    // check if we still have an active notification, if so, update that one
    const auto it = m_notifications.constFind(resId);
    if (it == m_notifications.cend()) {
        auto n = KNotification::event(KNotification::Notification, NotificationHelper::title(ld), NotificationHelper::message(ld), QLatin1String("clock"));
        m_notifications.insert(resId, n);
    } else {
        it.value()->setTitle(NotificationHelper::title(ld));
        it.value()->setText(NotificationHelper::message(ld));
        it.value()->update();
    }
}

void LiveDataManager::updateArrivalData(const KPublicTransport::Departure &arr, const QString &resId)
{
    // check if we can update static information in the reservation with what we received
    const auto res = m_resMgr->reservation(resId);
    if (JsonLd::isA<TrainReservation>(res)) {
        auto newRes = res.value<TrainReservation>();
        auto trip = res.value<TrainReservation>().reservationFor().value<TrainTrip>();
        trip.setArrivalStation(PublicTransport::mergeStation(trip.arrivalStation(), arr.stopPoint()));
        if (trip.arrivalPlatform().isEmpty() && !arr.scheduledPlatform().isEmpty()) {
            trip.setArrivalPlatform(arr.scheduledPlatform());
        }
        newRes.setReservationFor(trip);

        if (res.value<TrainReservation>() != newRes) {
            m_resMgr->updateReservation(resId, newRes);
        }
    }

    emit arrivalUpdated(resId);
}

void LiveDataManager::updateDepartureData(const KPublicTransport::Departure &dep, const QString &resId)
{
    // check if we can update static information in the reservation with what we received
    const auto res = m_resMgr->reservation(resId);
    if (JsonLd::isA<TrainReservation>(res)) {
        auto newRes = res.value<TrainReservation>();
        auto trip = res.value<TrainReservation>().reservationFor().value<TrainTrip>();
        trip.setDepartureStation(PublicTransport::mergeStation(trip.departureStation(), dep.stopPoint()));
        if (trip.departurePlatform().isEmpty() && !dep.scheduledPlatform().isEmpty()) {
            trip.setDeparturePlatform(dep.scheduledPlatform());
        }
        newRes.setReservationFor(trip);

        if (res.value<TrainReservation>() != newRes) {
            m_resMgr->updateReservation(resId, newRes);
        }
    }

    emit departureUpdated(resId);
}

QDateTime LiveDataManager::departureTime(const QString &resId, const QVariant &res) const
{
    if (JsonLd::isA<TrainReservation>(res)) {
        const auto &dep = departure(resId);
        if (dep.hasExpectedDepartureTime()) {
            return dep.expectedDepartureTime();
        }
    }

    return SortUtil::startDateTime(res);
}

QDateTime LiveDataManager::arrivalTime(const QString &resId, const QVariant &res) const
{
    if (JsonLd::isA<TrainReservation>(res)) {
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
    emit departureUpdated(resId);
    emit arrivalUpdated(resId);
}

bool LiveDataManager::isRelevant(const QString &resId) const
{
    const auto res = m_resMgr->reservation(resId);
    // we only care about transit reservations
    if (!JsonLd::canConvert<Reservation>(res) || !LocationUtil::isLocationChange(res)) {
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
        if (!isDepartureForReservation(res, (*dataIt).departure)) {
            (*dataIt).departure = {};
            (*dataIt).departureTimestamp = {};
            (*dataIt).store(resId, LiveData::Departure);
            emit departureUpdated(resId);
        }
        if (!isArrivalForReservation(res, (*dataIt).arrival)) {
            (*dataIt).arrival = {};
            (*dataIt).arrivalTimestamp = {};
            (*dataIt).store(resId, LiveData::Arrival);
            emit arrivalUpdated(resId);
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

    LiveData::remove(resId);
    m_data.remove(resId);
    // TODO remove active notification
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
            it = m_reservations.erase(it);
            continue;
        }
        ++it;

        if (!force && nextPollTimeForReservation(batchId) > 60 * 1000) {
            // data is still "fresh" according to the poll policy
            continue;
        }

        if (JsonLd::isA<TrainReservation>(res)) {
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

struct {
    int distance; // secs
    int pollInterval; // secs
} static const pollIntervalTable[] = {
    { 3600, 5*60 }, // for <1h we poll every 5 minutes
    { 4 * 3600, 15 * 60 }, // for <4h we poll every 15 minutes
    { 24 * 3600, 3600 }, // for <1d we poll once per hour
    { 4 * 24 * 3600, 24 * 3600 }, // for <4d we poll once per day
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
        ? (24 * 3600) // no poll yet == long time ago
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
    Q_UNUSED(passId);
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
