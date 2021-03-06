/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "timelinedelegatecontroller.h"

#include "constants.h"
#include "livedatamanager.h"
#include "locationhelper.h"
#include "logging.h"
#include "reservationmanager.h"
#include "publictransport.h"
#include "transfer.h"
#include "transfermanager.h"

#include <KItinerary/Flight>
#include <KItinerary/LocationUtil>
#include <KItinerary/SortUtil>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/Journey>

#include <QJSEngine>
#include <QJSValue>

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QPointF>
#include <QTimer>
#include <QTimeZone>

QTimer* TimelineDelegateController::s_currentTimer = nullptr;
int TimelineDelegateController::s_progressRefCount = 0;
QTimer* TimelineDelegateController::s_progressTimer = nullptr;

using namespace KItinerary;

TimelineDelegateController::TimelineDelegateController(QObject *parent)
    : QObject(parent)
{
    if (!s_currentTimer) {
        s_currentTimer = new QTimer(QCoreApplication::instance());
        s_currentTimer->setSingleShot(true);
        s_currentTimer->setTimerType(Qt::VeryCoarseTimer);
    }
    connect(s_currentTimer, &QTimer::timeout, this, [this]() { checkForUpdate(m_batchId); });

    connect(this, &TimelineDelegateController::contentChanged, this, &TimelineDelegateController::connectionWarningChanged);
}

TimelineDelegateController::~TimelineDelegateController() = default;

QObject* TimelineDelegateController::reservationManager() const
{
    return m_resMgr;
}

void TimelineDelegateController::setReservationManager(QObject *resMgr)
{
    if (m_resMgr == resMgr) {
        return;
    }

    m_resMgr = qobject_cast<ReservationManager*>(resMgr);
    Q_EMIT setupChanged();
    Q_EMIT contentChanged();
    Q_EMIT departureChanged();
    Q_EMIT arrivalChanged();
    Q_EMIT previousLocationChanged();

    connect(m_resMgr, &ReservationManager::batchChanged, this, &TimelineDelegateController::batchChanged);
    connect(m_resMgr, &ReservationManager::batchContentChanged, this, &TimelineDelegateController::batchChanged);
    // ### could be done more efficiently
    connect(m_resMgr, &ReservationManager::batchAdded, this, &TimelineDelegateController::previousLocationChanged);
    connect(m_resMgr, &ReservationManager::batchRemoved, this, &TimelineDelegateController::previousLocationChanged);

    checkForUpdate(m_batchId);
}

QObject* TimelineDelegateController::liveDataManager() const
{
    return m_liveDataMgr;
}

void TimelineDelegateController::setLiveDataManager(QObject* liveDataMgr)
{
    if (m_liveDataMgr == liveDataMgr) {
        return;
    }

    m_liveDataMgr = qobject_cast<LiveDataManager*>(liveDataMgr);
    Q_EMIT setupChanged();
    Q_EMIT departureChanged();
    Q_EMIT arrivalChanged();

    connect(m_liveDataMgr, &LiveDataManager::arrivalUpdated, this, &TimelineDelegateController::checkForUpdate);
    connect(m_liveDataMgr, &LiveDataManager::departureUpdated, this, &TimelineDelegateController::checkForUpdate);
    connect(m_liveDataMgr, &LiveDataManager::arrivalUpdated, this, [this](const auto &batchId) {
        if (batchId == m_batchId) {
            Q_EMIT arrivalChanged();
        }
    });
    connect(m_liveDataMgr, &LiveDataManager::departureUpdated, this, [this](const auto &batchId) {
        if (batchId == m_batchId) {
            Q_EMIT departureChanged();
        }
    });

    checkForUpdate(m_batchId);
}

QObject* TimelineDelegateController::transferManager() const
{
    return m_transferMgr;
}

void TimelineDelegateController::setTransferManager(QObject *transferMgr)
{
    if (m_transferMgr == transferMgr) {
        return;
    }

    m_transferMgr = qobject_cast<TransferManager*>(transferMgr);
    Q_EMIT setupChanged();
}

QString TimelineDelegateController::batchId() const
{
    return m_batchId;
}

void TimelineDelegateController::setBatchId(const QString &batchId)
{
    if (m_batchId == batchId)
        return;

    m_batchId = batchId;
    Q_EMIT batchIdChanged();
    Q_EMIT contentChanged();
    Q_EMIT departureChanged();
    Q_EMIT arrivalChanged();
    Q_EMIT previousLocationChanged();
    checkForUpdate(batchId);
}

bool TimelineDelegateController::isCurrent() const
{
    return m_isCurrent;
}

void TimelineDelegateController::setCurrent(bool current, const QVariant &res)
{
    if (current == m_isCurrent) {
        return;
    }

    m_isCurrent = current;
    Q_EMIT currentChanged();

    if (!LocationUtil::isLocationChange(res)) {
        return;
    }

    if (!s_progressTimer && m_isCurrent) {
        s_progressTimer = new QTimer(QCoreApplication::instance());
        s_progressTimer->setInterval(std::chrono::minutes(1));
        s_progressTimer->setTimerType(Qt::VeryCoarseTimer);
        s_progressTimer->setSingleShot(false);
    }

    if (m_isCurrent) {
        connect(s_progressTimer, &QTimer::timeout, this, &TimelineDelegateController::progressChanged);
        if (s_progressRefCount++ == 0) {
            s_progressTimer->start();
        }
    } else {
        disconnect(s_progressTimer, &QTimer::timeout, this, &TimelineDelegateController::progressChanged);
        if (--s_progressRefCount == 0) {
            s_progressTimer->stop();
        }
    }
}

float TimelineDelegateController::progress() const
{
    if (!m_resMgr || m_batchId.isEmpty() || !m_isCurrent) {
        return 0.0f;
    }

    const auto res = m_resMgr->reservation(m_batchId);
    const auto startTime = liveStartDateTime(res);
    const auto endTime = liveEndDateTime(res);

    const auto tripLength = startTime.secsTo(endTime);
    if (tripLength <= 0) {
        return 0.0f;
    }
    const auto progress = startTime.secsTo(QDateTime::currentDateTime());

    return std::min(std::max(0.0f, (float)progress / (float)tripLength), 1.0f);
}

KPublicTransport::Stopover TimelineDelegateController::arrival() const
{
    if (!m_liveDataMgr || m_batchId.isEmpty()) {
        return {};
    }
    return m_liveDataMgr->arrival(m_batchId);
}

KPublicTransport::Stopover TimelineDelegateController::departure() const
{
    if (!m_liveDataMgr || m_batchId.isEmpty()) {
        return {};
    }
    return m_liveDataMgr->departure(m_batchId);
}

KPublicTransport::JourneySection TimelineDelegateController::journey() const
{
    if (!m_liveDataMgr || m_batchId.isEmpty()) {
        return {};
    }
    return m_liveDataMgr->journey(m_batchId);
}

void TimelineDelegateController::checkForUpdate(const QString& batchId)
{
    if (!m_resMgr || m_batchId.isEmpty()) {
        setCurrent(false);
        return;
    }
    if (batchId != m_batchId) {
        return;
    }

    const auto res = m_resMgr->reservation(batchId);
    if (!LocationUtil::isLocationChange(res)) {
        return;
    }

    const auto now = QDateTime::currentDateTime();
    const auto startTime = relevantStartDateTime(res);
    const auto endTime = liveEndDateTime(res);

    setCurrent(startTime < now && now < endTime, res);

    if (now < startTime) {
        scheduleNextUpdate(std::chrono::seconds(now.secsTo(startTime) + 1));
    } else if (now < endTime) {
        scheduleNextUpdate(std::chrono::seconds(now.secsTo(endTime) + 1));
    }
}

QDateTime TimelineDelegateController::relevantStartDateTime(const QVariant &res) const
{
    auto startTime = SortUtil::startDateTime(res);
    if (JsonLd::isA<FlightReservation>(res)) {
        const auto flight = res.value<FlightReservation>().reservationFor().value<Flight>();
        if (flight.boardingTime().isValid()) {
            startTime = flight.boardingTime();
        }
    } else if (JsonLd::isA<TrainReservation>(res) || JsonLd::isA<BusReservation>(res)) {
        startTime = startTime.addSecs(-5 * 60);
    }

    return startTime;
}

QDateTime TimelineDelegateController::liveStartDateTime(const QVariant& res) const
{
    if (m_liveDataMgr) {
        const auto dep = m_liveDataMgr->departure(m_batchId);
        if (dep.expectedDepartureTime().isValid()) {
            return dep.expectedDepartureTime();
        }
    }
    return SortUtil::startDateTime(res);
}

QDateTime TimelineDelegateController::liveEndDateTime(const QVariant& res) const
{
    if (m_liveDataMgr) {
        const auto arr = m_liveDataMgr->arrival(m_batchId);
        if (arr.expectedArrivalTime().isValid()) {
            return arr.expectedArrivalTime();
        }
    }
    return SortUtil::endDateTime(res);
}

void TimelineDelegateController::scheduleNextUpdate(std::chrono::milliseconds ms)
{
    if (s_currentTimer->isActive() && s_currentTimer->remainingTimeAsDuration() < ms) {
        return;
    }
    s_currentTimer->start(ms);
}

void TimelineDelegateController::batchChanged(const QString& batchId)
{
    if (batchId != m_batchId || m_batchId.isEmpty()) {
        return;
    }
    checkForUpdate(batchId);
    Q_EMIT contentChanged();
    Q_EMIT arrivalChanged();
    Q_EMIT departureChanged();
    Q_EMIT previousLocationChanged();
}

QVariant TimelineDelegateController::previousLocation() const
{
    if (m_batchId.isEmpty() || !m_resMgr) {
        return {};
    }

    const auto prevBatch = m_resMgr->previousBatch(m_batchId);
    if (prevBatch.isEmpty()) {
        return {};
    }

    const auto res = m_resMgr->reservation(prevBatch);
    auto endTime = SortUtil::endDateTime(res);
    if (m_liveDataMgr) {
        const auto arr = m_liveDataMgr->arrival(prevBatch);
        if (arr.hasExpectedArrivalTime()) {
            endTime = arr.expectedArrivalTime();
        }
    }

    if (endTime < QDateTime::currentDateTime()) {
        // past event, we can use GPS rather than predict our location from the itinerary
        return {};
    }

    if (LocationUtil::isLocationChange(res)) {
        return LocationUtil::arrivalLocation(res);
    } else {
        return LocationUtil::location(res);
    }
}

QDateTime TimelineDelegateController::effectiveEndTime() const
{
    if (!m_resMgr || m_batchId.isEmpty()) {
        return {};
    }

    const auto arr = arrival();
    if (arr.hasExpectedArrivalTime()) {
        return arr.expectedArrivalTime();
    }
    return SortUtil::endDateTime(m_resMgr->reservation(m_batchId));
}

bool TimelineDelegateController::isLocationChange() const
{
    if (!m_resMgr || m_batchId.isEmpty()) {
        return false;
    }

    const auto res = m_resMgr->reservation(m_batchId);
    return LocationUtil::isLocationChange(res);
}

bool TimelineDelegateController::isPublicTransport() const
{
    if (!m_resMgr || m_batchId.isEmpty()) {
        return false;
    }

    const auto res = m_resMgr->reservation(m_batchId);
    return LocationUtil::isLocationChange(res) && !JsonLd::isA<RentalCarReservation>(res);
}

static bool isJourneyCandidate(const QVariant &res)
{
    // TODO do we really need to constrain this to trains/buses? a long distance train can be a suitable alternative for a missed short distance flight for example
    return LocationUtil::isLocationChange(res) && (JsonLd::isA<TrainReservation>(res) || JsonLd::isA<BusReservation>(res));
}

static bool isLayover(const QVariant &res1, const QVariant &res2)
{
    if (!LocationUtil::isLocationChange(res1) || !LocationUtil::isLocationChange(res2)) {
        return false;
    }

    const auto arrDt = SortUtil::endDateTime(res1);
    const auto depDt = SortUtil::startDateTime(res2);
    const auto layoverTime = arrDt.secsTo(depDt);
    if (layoverTime < 0 || layoverTime > Constants::MaximumLayoverTime.count()) {
        return false;
    }

    return LocationUtil::isSameLocation(LocationUtil::arrivalLocation(res1), LocationUtil::departureLocation(res2), LocationUtil::WalkingDistance);
}

KPublicTransport::JourneyRequest TimelineDelegateController::journeyRequest() const
{
    if (!m_resMgr || m_batchId.isEmpty()) {
        return {};
    }

    const auto res = m_resMgr->reservation(m_batchId);
    if (!isJourneyCandidate(res)) {
        return {};
    }

    KPublicTransport::JourneyRequest req;
    req.setFrom(PublicTransport::locationFromPlace(LocationUtil::departureLocation(res), res));
    req.setTo(PublicTransport::locationFromPlace(LocationUtil::arrivalLocation(res), res));
    req.setDateTime(std::max(QDateTime::currentDateTime(), SortUtil::startDateTime(res)));
    req.setDateTimeMode(KPublicTransport::JourneyRequest::Departure);
    req.setDownloadAssets(true);
    req.setIncludeIntermediateStops(true);
    req.setIncludePaths(true);

    // find full journey by looking at subsequent elements
    auto prevRes = res;
    auto prevBatchId = m_batchId;
    while (true) {
        auto endBatchId = m_resMgr->nextBatch(prevBatchId);
        auto endRes = m_resMgr->reservation(endBatchId);
        if (!isJourneyCandidate(endRes) || !isLayover(prevRes, endRes)) {
            break;
        }

        req.setTo(PublicTransport::locationFromPlace(LocationUtil::arrivalLocation(endRes), endRes));
        prevRes = endRes;
        prevBatchId = endBatchId;
    }

    return req;
}

void TimelineDelegateController::applyJourney(const QVariant &journey)
{
    if (!m_resMgr || m_batchId.isEmpty()) {
        return;
    }

    const auto jny = journey.value<KPublicTransport::Journey>();
    std::vector<KPublicTransport::JourneySection> sections;
    std::copy_if(jny.sections().begin(), jny.sections().end(), std::back_inserter(sections), [](const auto &section) {
        return section.mode() == KPublicTransport::JourneySection::PublicTransport;
    });
    if (sections.empty()) {
        return;
    }

    // find all batches we are replying here (same logic as in journeyRequest)
    std::vector<QString> oldBatches({m_batchId});
    {
        auto prevRes = m_resMgr->reservation(m_batchId);
        auto prevBatchId = m_batchId;
        while (true) {
            auto endBatchId = m_resMgr->nextBatch(prevBatchId);
            auto endRes = m_resMgr->reservation(endBatchId);
            qDebug() << endRes << isJourneyCandidate(endRes) << isLayover(prevRes, endRes);
            if (!isJourneyCandidate(endRes) || !isLayover(prevRes, endRes)) {
                break;
            }

            oldBatches.push_back(endBatchId);
            prevRes = endRes;
            prevBatchId = endBatchId;
        }
    }
    qCDebug(Log) << "Affected batches:" << oldBatches;

    // align sections with affected batches, by type, and insert/update accordingly
    auto it = oldBatches.begin();
    for (const auto &section : sections) {
        QVariant oldRes;
        if (it != oldBatches.end()) {
            oldRes = m_resMgr->reservation(*it);
        }

        // same type -> update the existing one
        if (PublicTransport::isSameMode(oldRes, section)) {
            const auto resIds = m_resMgr->reservationsForBatch(*it);
            for (const auto &resId : resIds) {
                auto res = m_resMgr->reservation(resId);
                res = PublicTransport::applyJourneySection(res, section);
                m_resMgr->updateReservation(resId, res);
                m_liveDataMgr->setJourney(resId, section);
            }
            ++it;
        } else {
            const auto res = PublicTransport::reservationFromJourneySection(section);
            const auto resId = m_resMgr->addReservation(res);
            m_liveDataMgr->setJourney(resId, section);
        }
    }

    // remove left over reservations
    for (; it != oldBatches.end(); ++it) {
        m_resMgr->removeBatch(*it);
    }
}

bool TimelineDelegateController::connectionWarning() const
{
    if (!m_resMgr || m_batchId.isEmpty() || !m_liveDataMgr) {
        return false;
    }

    const auto curRes = m_resMgr->reservation(m_batchId);
    if (!LocationUtil::isLocationChange(curRes)) {
        return false;
    }

    // if the current item has canceled departure/arrival, warn as well
    if (departure().disruptionEffect() == KPublicTransport::Disruption::NoService || arrival().disruptionEffect() == KPublicTransport::Disruption::NoService) {
        return true;
    }

    const auto prevResId = m_resMgr->previousBatch(m_batchId);
    const auto prevRes = m_resMgr->reservation(prevResId);
    if (!LocationUtil::isLocationChange(prevRes)) {
        return false;
    }

    const auto prevArr = m_liveDataMgr->arrival(prevResId);
    const auto prevArrDt = std::max(SortUtil::endDateTime(prevRes), prevArr.expectedArrivalTime());

    const auto curDepDt = std::max(SortUtil::startDateTime(curRes), departure().expectedDepartureTime());
    return curDepDt < prevArrDt;
}

bool TimelineDelegateController::isCanceled() const
{
    if (!m_resMgr || m_batchId.isEmpty()) {
        return false;
    }

    const auto res = m_resMgr->reservation(m_batchId);
    if (!JsonLd::canConvert<Reservation>(res)) {
        return false;
    }
    return JsonLd::convert<Reservation>(res).reservationStatus() == Reservation::ReservationCancelled;
}

QJSValue TimelineDelegateController::arrivalMapArguments() const
{
    const auto engine = qjsEngine(this);
    if (!engine || !m_resMgr || m_batchId.isEmpty() || !m_transferMgr || !m_liveDataMgr) {
        return {};
    }

    const auto res = m_resMgr->reservation(m_batchId);
    if (!LocationUtil::isLocationChange(res)) {
        return {};
    }

    auto args = engine->newObject();
    const auto arrLoc = LocationUtil::arrivalLocation(res);
    args.setProperty(QStringLiteral("placeName"), LocationUtil::name(arrLoc));
    args.setProperty(QStringLiteral("region"), LocationHelper::regionCode(arrLoc));

    // arrival location
    const auto arr = arrival();
    auto arrPlatform = arr.hasExpectedPlatform() ? arr.expectedPlatform() : arr.scheduledPlatform();
    if (arrPlatform.isEmpty() && JsonLd::isA<TrainReservation>(res)) {
        arrPlatform = res.value<TrainReservation>().reservationFor().value<TrainTrip>().arrivalPlatform();
    }
    args.setProperty(QStringLiteral("arrivalPlatformName"), arrPlatform);
    // there is no arrival gate property (yet)

    // arrival time
    auto arrTime = arr.hasExpectedArrivalTime() ? arr.expectedArrivalTime() : arr.scheduledArrivalTime();
    if (!arrTime.isValid()) {
        arrTime = SortUtil::endDateTime(res);
    }
    args.setProperty(QStringLiteral("beginTime"), engine->toScriptValue(arrTime));
    if (arrTime.timeSpec() == Qt::TimeZone) {
        args.setProperty(QStringLiteral("timeZone"), QString::fromUtf8(arrTime.timeZone().id()));
    }

    // look for departure for a following transfer
    const auto transfer = m_transferMgr->transfer(m_batchId, Transfer::After);
    if (transfer.state() == Transfer::Selected) {
        const auto dep = PublicTransport::firstTransportSection(transfer.journey());
        args.setProperty(QStringLiteral("departurePlatformName"), dep.hasExpectedDeparturePlatform() ? dep.expectedDeparturePlatform() : dep.scheduledDeparturePlatform());
        args.setProperty(QStringLiteral("endTime"), engine->toScriptValue(dep.hasExpectedDepartureTime() ? dep.expectedDepartureTime() : dep.scheduledDepartureTime()));
        return args;
    }

    // ... or layover
    const auto nextResId = m_resMgr->nextBatch(m_batchId);
    const auto nextRes = m_resMgr->reservation(nextResId);
    if (!isLayover(res, nextRes)) {
        return args;
    }
    const auto dep = m_liveDataMgr->departure(nextResId);
    auto depPlatform = dep.hasExpectedPlatform() ? dep.expectedPlatform() : dep.scheduledPlatform();
    if (depPlatform.isEmpty() && JsonLd::isA<TrainReservation>(nextRes)) {
        depPlatform = nextRes.value<TrainReservation>().reservationFor().value<TrainTrip>().departurePlatform();
    }
    args.setProperty(QStringLiteral("departurePlatformName"), depPlatform);
    if (JsonLd::isA<FlightReservation>(nextRes)) {
        args.setProperty(QStringLiteral("departureGateName"), nextRes.value<FlightReservation>().reservationFor().value<Flight>().departureGate());
    }

    auto depTime = dep.hasExpectedDepartureTime() ? dep.expectedDepartureTime() : dep.scheduledDepartureTime();
    if (!depTime.isValid()) {
        depTime = SortUtil::startDateTime(nextRes);
    }
    args.setProperty(QStringLiteral("endTime"), engine->toScriptValue(depTime));

    return args;
}

QJSValue TimelineDelegateController::departureMapArguments() const
{
    const auto engine = qjsEngine(this);
    if (!engine || !m_resMgr || m_batchId.isEmpty() || !m_transferMgr || !m_liveDataMgr) {
        return {};
    }

    const auto res = m_resMgr->reservation(m_batchId);
    if (!LocationUtil::isLocationChange(res)) {
        return {};
    }

    auto args = engine->newObject();
    const auto depLoc = LocationUtil::departureLocation(res);
    args.setProperty(QStringLiteral("placeName"), LocationUtil::name(depLoc));
    args.setProperty(QStringLiteral("region"), LocationHelper::regionCode(depLoc));

    // departure location
    const auto dep = departure();
    auto depPlatform = dep.hasExpectedPlatform() ? dep.expectedPlatform() : dep.scheduledPlatform();
    if (depPlatform.isEmpty() && JsonLd::isA<TrainReservation>(res)) {
        depPlatform = res.value<TrainReservation>().reservationFor().value<TrainTrip>().departurePlatform();
    }
    args.setProperty(QStringLiteral("departurePlatformName"), depPlatform);
    if (JsonLd::isA<FlightReservation>(res)) {
        args.setProperty(QStringLiteral("departureGateName"), res.value<FlightReservation>().reservationFor().value<Flight>().departureGate());
    }

    // departure time
    auto depTime = dep.hasExpectedDepartureTime() ? dep.expectedDepartureTime() : dep.scheduledDepartureTime();
    if (!depTime.isValid()) {
        depTime = SortUtil::startDateTime(res);
    }
    args.setProperty(QStringLiteral("endTime"), engine->toScriptValue(depTime));
    if (depTime.timeSpec() == Qt::TimeZone) {
        args.setProperty(QStringLiteral("timeZone"), QString::fromUtf8(depTime.timeZone().id()));
    }

    // look for arrival for a preceding transfer
    const auto transfer = m_transferMgr->transfer(m_batchId, Transfer::Before);
    if (transfer.state() == Transfer::Selected) {
        const auto arr = PublicTransport::lastTransportSection(transfer.journey());
        args.setProperty(QStringLiteral("arrivalPlatformName"), arr.hasExpectedArrivalPlatform() ? arr.expectedArrivalPlatform() : arr.scheduledArrivalPlatform());
        args.setProperty(QStringLiteral("beginTime"), engine->toScriptValue(arr.hasExpectedArrivalTime() ? arr.expectedArrivalTime() : arr.scheduledArrivalTime()));
        return args;
    }

    // ... or layover
    const auto prevResId = m_resMgr->previousBatch(m_batchId);
    const auto prevRes = m_resMgr->reservation(prevResId);
    if (!isLayover(prevRes, res)) {
        return args;
    }
    const auto arr = m_liveDataMgr->arrival(prevResId);
    auto arrPlatform = arr.hasExpectedPlatform() ? arr.expectedPlatform() : arr.scheduledPlatform();
    if (arrPlatform.isEmpty() && JsonLd::isA<TrainReservation>(prevRes)) {
        arrPlatform = prevRes.value<TrainReservation>().reservationFor().value<TrainTrip>().arrivalPlatform();
    }
    args.setProperty(QStringLiteral("arrivalPlatformName"), arrPlatform);
    // there is no arrival gate property (yet)

    auto arrTime = arr.hasExpectedArrivalTime() ? arr.expectedArrivalTime() : arr.scheduledArrivalTime();
    if (!arrTime.isValid()) {
        arrTime = SortUtil::endDateTime(prevRes);
    }
    args.setProperty(QStringLiteral("beginTime"), engine->toScriptValue(arrTime));

    return args;
}

#include "moc_timelinedelegatecontroller.cpp"
