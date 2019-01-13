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

#include "config-itinerary.h"

#include "livedatamanager.h"
#include "logging.h"
#include "pkpassmanager.h"
#include "reservationmanager.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/DepartureReply>
#include <KPublicTransport/DepartureRequest>
#include <KPublicTransport/Location>
#include <KPublicTransport/Manager>

#ifdef HAVE_NOTIFICATIONS
#include <KNotifications/KNotification>
#endif

#include <KLocalizedString>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QVector>

using namespace KItinerary;

LiveDataManager::LiveDataManager(QObject *parent)
    : QObject(parent)

{
    m_ptMgr.reset(new KPublicTransport::Manager);

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

    loadPublicTransportData();
    m_pollTimer.setInterval(nextPollTime());
}

void LiveDataManager::setPkPassManager(PkPassManager *pkPassMgr)
{
    m_pkPassMgr = pkPassMgr;
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

void LiveDataManager::setAllowInsecureServices(bool allowInsecure)
{
    m_ptMgr->setAllowInsecureBackends(allowInsecure);
}

QVariant LiveDataManager::arrival(const QString &resId)
{
    return QVariant::fromValue(m_arrivals.value(resId).change);
}

QVariant LiveDataManager::departure(const QString &resId)
{
    return QVariant::fromValue(m_departures.value(resId).change);
}

void LiveDataManager::checkForUpdates()
{
    pollForUpdates(true);
}

static QString stripSpecial(const QString &str)
{
    QString res;
    res.reserve(str.size());
    std::copy_if(str.begin(), str.end(), std::back_inserter(res), [](const auto c) {
        return c.isLetter() || c.isDigit();
    });
    return res;
}

static bool isSameLine(const QString &lineName, const QString &trainName, const QString &trainNumber)
{
    const auto lhs = stripSpecial(lineName);
    const auto rhs = stripSpecial(trainName + trainNumber);
    return lhs.compare(rhs, Qt::CaseInsensitive) == 0;
}

static KPublicTransport::Location locationFromStation(const TrainStation &station)
{
    using namespace KPublicTransport;
    Location loc;
    loc.setName(station.name());
    loc.setCoordinate(station.geo().latitude(), station.geo().longitude());
    if (!station.identifier().isEmpty()) {
        const auto idSplit = station.identifier().split(QLatin1Char(':'));
        if (idSplit.size() == 2) {
            loc.setIdentifier(idSplit.at(0), idSplit.at(1));
        }
    }
    return loc;
}

void LiveDataManager::checkTrainTrip(const QVariant &res, const QString& resId)
{
    Q_ASSERT(JsonLd::isA<TrainReservation>(res));
    const auto trip = res.value<TrainReservation>().reservationFor().value<TrainTrip>();

    qCDebug(Log) << trip.trainName() << trip.trainNumber() << trip.departureTime();
    using namespace KPublicTransport;

    if (!hasDeparted(resId, res)) {
        DepartureRequest req(locationFromStation(trip.departureStation()));
        req.setDateTime(trip.departureTime());
        auto reply = m_ptMgr->queryDeparture(req);
        connect(reply, &Reply::finished, this, [this, trip, resId, reply]() {
            reply->deleteLater();
            if (reply->error() != Reply::NoError) {
                qCDebug(Log) << reply->error() << reply->errorString();
                return;
            }

            for (const auto &dep : reply->result()) {
                qCDebug(Log) << "Got departure information:" << dep.route().line().name() << dep.scheduledDepartureTime() << "for" << trip.trainNumber();
                if (dep.scheduledDepartureTime() != trip.departureTime() || !isSameLine(dep.route().line().name(), trip.trainName(), trip.trainNumber())) {
                    continue;
                }
                qCDebug(Log) << "Found departure information:" << dep.route().line().name() << dep.expectedPlatform() << dep.expectedDepartureTime();
                updateDepartureData(dep, resId);
                break;
            }
        });
    }

    if (!hasArrived(resId, res)) {
        DepartureRequest req(locationFromStation(trip.arrivalStation()));
        req.setMode(DepartureRequest::QueryArrival);
        req.setDateTime(trip.arrivalTime());
        auto reply = m_ptMgr->queryDeparture(req);
        connect(reply, &Reply::finished, this, [this, trip, resId, reply]() {
            reply->deleteLater();
            if (reply->error() != Reply::NoError) {
                qCDebug(Log) << reply->error() << reply->errorString();
                return;
            }

            for (const auto &arr : reply->result()) {
                qCDebug(Log) << "Got arrival information:" << arr.route().line().name() << arr.scheduledArrivalTime() << "for" << trip.trainNumber();
                if (arr.scheduledArrivalTime() != trip.arrivalTime() || !isSameLine(arr.route().line().name(), trip.trainName(), trip.trainNumber())) {
                    continue;
                }
                qCDebug(Log) << "Found arrival information:" << arr.route().line().name() << arr.expectedPlatform() << arr.expectedDepartureTime();
                updateArrivalData(arr, resId);
                break;
            }
        });
    }
}

void LiveDataManager::updateArrivalData(const KPublicTransport::Departure &arr, const QString &resId)
{
    const auto oldArr = m_arrivals.value(resId).change;
    m_arrivals.insert(resId, {arr, QDateTime::currentDateTimeUtc()});
    storePublicTransportData(resId, arr, QStringLiteral("arrival"));

    // check if we can update static information in the reservation with what we received
    const auto res = m_resMgr->reservation(resId);
    if (JsonLd::isA<TrainReservation>(res)) {
        auto newRes = res.value<TrainReservation>();
        auto trip = res.value<TrainReservation>().reservationFor().value<TrainTrip>();
        auto station = trip.arrivalStation();
        if (!station.geo().isValid() && arr.stopPoint().hasCoordinate()) {
            station.setGeo(GeoCoordinates{arr.stopPoint().latitude(), arr.stopPoint().longitude()});
            trip.setArrivalStation(station);
        }
        if (trip.arrivalPlatform().isEmpty() && !arr.scheduledPlatform().isEmpty()) {
            trip.setArrivalPlatform(arr.scheduledPlatform());
        }
        newRes.setReservationFor(trip);

        if (res.value<TrainReservation>() != newRes) {
            m_resMgr->updateReservation(resId, newRes);
        }
    }

    // check if something changed relevant for notifications
    if (oldArr.arrivalDelay() == arr.arrivalDelay() && oldArr.expectedPlatform() == arr.expectedPlatform()) {
        return;
    }

#ifdef HAVE_NOTIFICATIONS
    // check if something worth notifying changed
    // ### we could do that even more clever by skipping distant future changes
    if (std::abs(oldArr.arrivalDelay() - arr.arrivalDelay()) > 2) {
        KNotification::event(KNotification::Notification,
            i18n("Delayed arrival on %1", arr.route().line().name()),
            i18n("New arrival time is: %1", QLocale().toString(arr.expectedArrivalTime().time())),
            QLatin1String("clock"));
    }
#endif

    emit departureUpdated(resId);
}

void LiveDataManager::updateDepartureData(const KPublicTransport::Departure &dep, const QString &resId)
{
    const auto oldDep = m_departures.value(resId).change;
    m_departures.insert(resId, {dep, QDateTime::currentDateTimeUtc()});
    storePublicTransportData(resId, dep, QStringLiteral("departure"));

    // check if we can update static information in the reservation with what we received
    const auto res = m_resMgr->reservation(resId);
    if (JsonLd::isA<TrainReservation>(res)) {
        auto newRes = res.value<TrainReservation>();
        auto trip = res.value<TrainReservation>().reservationFor().value<TrainTrip>();
        auto station = trip.departureStation();
        if (!station.geo().isValid() && dep.stopPoint().hasCoordinate()) {
            station.setGeo(GeoCoordinates{dep.stopPoint().latitude(), dep.stopPoint().longitude()});
            trip.setDeparatureStation(station);
        }
        if (trip.departurePlatform().isEmpty() && !dep.scheduledPlatform().isEmpty()) {
            trip.setDeparturePlatform(dep.scheduledPlatform());
        }
        newRes.setReservationFor(trip);

        if (res.value<TrainReservation>() != newRes) {
            m_resMgr->updateReservation(resId, newRes);
        }
    }

    // check if something changed relevant for notification
    if (oldDep.departureDelay() == dep.departureDelay() && oldDep.expectedPlatform() == dep.expectedPlatform()) {
        return;
    }

#ifdef HAVE_NOTIFICATIONS
    // check if something worth notifying changed
    // ### we could do that even more clever by skipping distant future changes
    if (std::abs(oldDep.departureDelay() - dep.departureDelay()) > 2) {
        KNotification::event(KNotification::Notification,
            i18n("Delayed departure on %1", dep.route().line().name()),
            i18n("New departure time is: %1", QLocale().toString(dep.expectedDepartureTime().time())),
            QLatin1String("clock"));
    }

    if (oldDep.expectedPlatform() != dep.expectedPlatform() && dep.scheduledPlatform() != dep.expectedPlatform()) {
        KNotification::event(KNotification::Notification,
            i18n("Platform change on %1", dep.route().line().name()),
            i18n("New departure platform is: %1", dep.expectedPlatform()),
            QLatin1String("clock"));
    }
#endif

    emit departureUpdated(resId);
}

QDateTime LiveDataManager::departureTime(const QString &resId, const QVariant &res) const
{
    if (JsonLd::isA<TrainTrip>(res)) {
        const auto &dep = m_departures.value(resId).change;
        if (dep.hasExpectedDepartureTime()) {
            return dep.expectedDepartureTime();
        }
    }

    return SortUtil::startDateTime(res);
}

QDateTime LiveDataManager::arrivalTime(const QString &resId, const QVariant &res) const
{
    if (JsonLd::isA<TrainTrip>(res)) {
        const auto &arr = m_arrivals.value(resId).change;
        if (arr.hasExpectedArrivalTime()) {
            return arr.expectedArrivalTime();
        }
    }

    return SortUtil::endtDateTime(res);
}

bool LiveDataManager::hasDeparted(const QString &resId, const QVariant &res) const
{
    return departureTime(resId, res) < QDateTime::currentDateTime();
}

bool LiveDataManager::hasArrived(const QString &resId, const QVariant &res) const
{
    return arrivalTime(resId, res) < QDateTime::currentDateTime();
}

void LiveDataManager::loadPublicTransportData(const QString &prefix, QHash<QString, TrainChange> &data) const
{
    const auto basePath = QString(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/publictransport/"));
    QDirIterator it(basePath + prefix, QDir::Files | QDir::NoSymLinks);
    while (it.hasNext()) {
        it.next();
        const auto resId = it.fileInfo().baseName();
        if (std::find(m_reservations.begin(), m_reservations.end(), resId) == m_reservations.end()) {
            QDir(it.path()).remove(it.fileName());
        } else {
            QFile f(it.filePath());
            if (!f.open(QFile::ReadOnly)) {
                qCWarning(Log) << "Failed to load public transport file" << f.fileName() << f.errorString();
                continue;
            }
            data.insert(resId, {KPublicTransport::Departure::fromJson(QJsonDocument::fromJson(f.readAll()).object()), f.fileTime(QFile::FileModificationTime)});
        }
    }
}

void LiveDataManager::loadPublicTransportData()
{
    loadPublicTransportData(QStringLiteral("arrival"), m_arrivals);
    loadPublicTransportData(QStringLiteral("departure"), m_departures);
}

void LiveDataManager::storePublicTransportData(const QString &resId, const KPublicTransport::Departure &dep, const QString &type) const
{
    const auto basePath = QString(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/publictransport/")
        + type + QLatin1Char('/'));
    QDir().mkpath(basePath);

    QFile file(basePath + resId + QLatin1String(".json"));
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        qCWarning(Log) << "Failed to open public transport cache file:" << file.fileName() << file.errorString();
        return;
    }
    file.write(QJsonDocument(KPublicTransport::Departure::toJson(dep)).toJson());
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
    } else {
        m_reservations.erase(it);
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
            checkTrainTrip(res, batchId);
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

    const auto now = QDateTime::currentDateTime();
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
    const auto lastArrivalPoll = m_arrivals.value(resId).timestamp;
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
    auto dt = m_departures.value(batchId).timestamp;
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

#include "moc_livedatamanager.cpp"
