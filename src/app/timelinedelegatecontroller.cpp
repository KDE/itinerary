/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "timelinedelegatecontroller.h"

#include "calendarhelper.h"
#include "constants.h"
#include "documentmanager.h"
#include "livedatamanager.h"
#include "locationhelper.h"
#include "logging.h"
#include "publictransport.h"
#include "publictransportmatcher.h"
#include "reservationhelper.h"
#include "reservationmanager.h"
#include "transfer.h"
#include "transfermanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

#include <KItinerary/BusTrip>
#include <KItinerary/CalendarHandler>
#include <KItinerary/DocumentUtil>
#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/LocationUtil>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/Platform>
#include <KPublicTransport/PlatformLayout>
#include <KPublicTransport/Vehicle>

#include <QJSEngine>
#include <QJSValue>

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QPointF>
#include <QTimeZone>
#include <QTimer>

using namespace Qt::Literals;

QTimer *TimelineDelegateController::s_currentTimer = nullptr;
int TimelineDelegateController::s_progressRefCount = 0;
QTimer *TimelineDelegateController::s_progressTimer = nullptr;

using namespace KItinerary;

TimelineDelegateController::TimelineDelegateController(QObject *parent)
    : QObject(parent)
{
    if (!s_currentTimer) {
        s_currentTimer = new QTimer(QCoreApplication::instance());
        s_currentTimer->setSingleShot(true);
        s_currentTimer->setTimerType(Qt::VeryCoarseTimer);
    }
    connect(s_currentTimer, &QTimer::timeout, this, [this]() {
        checkForUpdate(m_batchId);
    });

    connect(this, &TimelineDelegateController::contentChanged, this, &TimelineDelegateController::connectionWarningChanged);
}

TimelineDelegateController::~TimelineDelegateController() = default;

ReservationManager *TimelineDelegateController::reservationManager() const
{
    return m_resMgr;
}

void TimelineDelegateController::setReservationManager(ReservationManager *resMgr)
{
    if (m_resMgr == resMgr) {
        return;
    }

    m_resMgr = resMgr;
    Q_EMIT setupChanged();
    Q_EMIT contentChanged();
    Q_EMIT departureChanged();
    Q_EMIT arrivalChanged();
    Q_EMIT journeyChanged();
    Q_EMIT previousLocationChanged();
    Q_EMIT layoutChanged();

    connect(m_resMgr, &ReservationManager::batchChanged, this, &TimelineDelegateController::batchChanged);
    connect(m_resMgr, &ReservationManager::batchContentChanged, this, &TimelineDelegateController::batchChanged);
    // ### could be done more efficiently
    connect(m_resMgr, &ReservationManager::batchAdded, this, &TimelineDelegateController::previousLocationChanged);
    connect(m_resMgr, &ReservationManager::batchRemoved, this, &TimelineDelegateController::previousLocationChanged);

    checkForUpdate(m_batchId);
}

LiveDataManager *TimelineDelegateController::liveDataManager() const
{
    return m_liveDataMgr;
}

void TimelineDelegateController::setLiveDataManager(LiveDataManager *liveDataMgr)
{
    if (m_liveDataMgr == liveDataMgr) {
        return;
    }

    m_liveDataMgr = liveDataMgr;
    Q_EMIT setupChanged();
    Q_EMIT departureChanged();
    Q_EMIT arrivalChanged();
    Q_EMIT journeyChanged();
    Q_EMIT layoutChanged();

    connect(m_liveDataMgr, &LiveDataManager::arrivalUpdated, this, &TimelineDelegateController::checkForUpdate);
    connect(m_liveDataMgr, &LiveDataManager::departureUpdated, this, &TimelineDelegateController::checkForUpdate);
    connect(m_liveDataMgr, &LiveDataManager::arrivalUpdated, this, [this](const auto &batchId) {
        if (batchId == m_batchId) {
            Q_EMIT arrivalChanged();
            Q_EMIT journeyChanged();
        }
    });
    connect(m_liveDataMgr, &LiveDataManager::departureUpdated, this, [this](const auto &batchId) {
        if (batchId == m_batchId) {
            Q_EMIT departureChanged();
            Q_EMIT journeyChanged();
        }
    });
    connect(m_liveDataMgr, &LiveDataManager::journeyUpdated, this, [this](const auto &batchId) {
        if (batchId == m_batchId) {
            Q_EMIT departureChanged();
            Q_EMIT arrivalChanged();
            Q_EMIT journeyChanged();
        }
    });

    checkForUpdate(m_batchId);
}

TransferManager *TimelineDelegateController::transferManager() const
{
    return m_transferMgr;
}

void TimelineDelegateController::setTransferManager(TransferManager *transferMgr)
{
    if (m_transferMgr == transferMgr) {
        return;
    }

    m_transferMgr = transferMgr;
    Q_EMIT setupChanged();
}

DocumentManager *TimelineDelegateController::documentManager() const
{
    return m_documentMgr;
}

void TimelineDelegateController::setDocumentManager(DocumentManager *documentMgr)
{
    if (m_documentMgr == documentMgr) {
        return;
    }

    m_documentMgr = documentMgr;
    Q_EMIT setupChanged();
    Q_EMIT contentChanged();
}

QString TimelineDelegateController::batchId() const
{
    return m_batchId;
}

void TimelineDelegateController::setBatchId(const QString &batchId)
{
    if (m_batchId == batchId) {
        return;
    }

    m_batchId = batchId;
    Q_EMIT batchIdChanged();
    Q_EMIT contentChanged();
    Q_EMIT departureChanged();
    Q_EMIT arrivalChanged();
    Q_EMIT journeyChanged();
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

void TimelineDelegateController::checkForUpdate(const QString &batchId)
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
    if (ReservationHelper::isCancelled(res)) {
        setCurrent(false);
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

QDateTime TimelineDelegateController::relevantStartDateTime(const QVariant &res)
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

QDateTime TimelineDelegateController::liveStartDateTime(const QVariant &res) const
{
    if (m_liveDataMgr) {
        const auto dep = m_liveDataMgr->departure(m_batchId);
        if (dep.expectedDepartureTime().isValid()) {
            return dep.expectedDepartureTime();
        }
    }
    return SortUtil::startDateTime(res);
}

QDateTime TimelineDelegateController::liveEndDateTime(const QVariant &res) const
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

void TimelineDelegateController::batchChanged(const QString &batchId)
{
    if (batchId != m_batchId || m_batchId.isEmpty()) {
        return;
    }
    checkForUpdate(batchId);
    Q_EMIT contentChanged();
    Q_EMIT arrivalChanged();
    Q_EMIT departureChanged();
    Q_EMIT journeyChanged();
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
    }
    return LocationUtil::location(res);
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
    // TODO do we really need to constrain this to trains/buses? a long distance train can be a suitable alternative for a missed short distance flight for
    // example
    return LocationUtil::isLocationChange(res) && (JsonLd::isA<TrainReservation>(res) || JsonLd::isA<BusReservation>(res));
}

static bool isLayover(const QVariant &res1, const QVariant &res2)
{
    if (!LocationUtil::isLocationChange(res1) || !LocationUtil::isLocationChange(res2) || ReservationHelper::isUnbound(res1)
        || ReservationHelper::isUnbound(res2)) {
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

KPublicTransport::JourneyRequest TimelineDelegateController::journeyRequestOne() const
{
    if (!m_resMgr || m_batchId.isEmpty() || !m_liveDataMgr) {
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
    PublicTransport::selectBackends(req, m_liveDataMgr->publicTransportManager(), res);
    return req;
}

KPublicTransport::JourneyRequest TimelineDelegateController::journeyRequestFull() const
{
    auto req = journeyRequestOne();
    if (!req.isValid()) {
        return {};
    }

    // find full journey by looking at subsequent elements
    auto prevRes = m_resMgr->reservation(m_batchId);
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

void TimelineDelegateController::applyJourney(const QVariant &journey, bool includeFollowing)
{
    if (!m_resMgr || !m_tripGroupMgr || m_batchId.isEmpty()) {
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
    if (includeFollowing) {
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

    // whatever we do here should not change trip grouping
    const auto tgId = m_tripGroupMgr->tripGroupIdForReservation(m_batchId);
    TripGroupingBlocker blocker(m_tripGroupMgr);

    // align sections with affected batches, by type, and insert/update accordingly
    auto it = oldBatches.begin();
    QString lastResId;
    QStringList groupResIds;
    for (const auto &section : sections) {
        QVariant oldRes;
        if (it != oldBatches.end()) {
            lastResId = *it;
            oldRes = m_resMgr->reservation(*it);
        }

        // same type -> update the existing one
        if (PublicTransportMatcher::isSameMode(oldRes, section)) {
            const auto resIds = m_resMgr->reservationsForBatch(*it);
            for (const auto &resId : resIds) {
                auto res = m_resMgr->reservation(resId);
                res = PublicTransport::applyJourneySection(res, section);
                m_resMgr->updateReservation(resId, res);
                m_liveDataMgr->setJourney(resId, section);
            }
            groupResIds.push_back(*it);
            ++it;
        } else {
            auto res = PublicTransport::reservationFromJourneySection(section);

            // copy ticket data from previous element
            // TODO this would need to be done for the entire batch!
            if (!lastResId.isEmpty()) {
                auto lastRes = m_resMgr->reservation(lastResId);
                JsonLdDocument::writeProperty(lastRes, "reservationFor", {});
                res = JsonLdDocument::apply(lastRes, res);
            }

            const auto resId = m_resMgr->addReservation(res);
            m_liveDataMgr->setJourney(resId, section);
            groupResIds.push_back(resId);
        }
    }

    if (!tgId.isEmpty() && !groupResIds.isEmpty() && !m_tripGroupMgr->tripGroup(tgId).name().isEmpty()) {
        m_tripGroupMgr->addToGroup(groupResIds, tgId);
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
    if (!LocationUtil::isLocationChange(curRes) || ReservationHelper::isCancelled(curRes)) {
        return false;
    }

    // if the current item has canceled departure/arrival, warn as well
    if (departure().disruptionEffect() == KPublicTransport::Disruption::NoService || arrival().disruptionEffect() == KPublicTransport::Disruption::NoService) {
        return true;
    }

    const auto prevResId = m_resMgr->previousBatch(m_batchId);
    const auto prevRes = m_resMgr->reservation(prevResId);
    if (!LocationUtil::isLocationChange(prevRes) || ReservationHelper::isCancelled(prevRes)) {
        return false;
    }

    const auto prevArr = m_liveDataMgr->arrival(prevResId);
    const auto prevArrDt = std::max(SortUtil::endDateTime(prevRes), prevArr.expectedArrivalTime());

    const auto curDepDt = std::max(SortUtil::startDateTime(curRes), departure().expectedDepartureTime());
    if (curDepDt.isValid() && prevArrDt.isValid()) {
        return curDepDt < prevArrDt;
    }

    return false;
}

bool TimelineDelegateController::isCanceled() const
{
    if (!m_resMgr || m_batchId.isEmpty()) {
        return false;
    }

    const auto res = m_resMgr->reservation(m_batchId);
    return ReservationHelper::isCancelled(res);
}

static void mapArgumentsForLocation(QJSValue &args, const QVariant &location, QJSEngine *engine)
{
    args.setProperty(u"placeName"_s, LocationUtil::name(location));
    args.setProperty(u"region"_s, LocationHelper::regionCode(location));

    const auto geo = LocationUtil::geo(location);
    args.setProperty(u"coordinate"_s, engine->toScriptValue(QPointF(geo.longitude(), geo.latitude())));
}

struct CoachData {
    QString coachName;
    KPublicTransport::VehicleSection::Class coachClass = KPublicTransport::VehicleSection::UnknownClass;
};

static CoachData coachDataForReservation(const QVariant &res)
{
    if (!JsonLd::isA<TrainReservation>(res)) {
        return {};
    }

    const auto trainRes = res.value<TrainReservation>();
    const auto seat = trainRes.reservedTicket().value<Ticket>().ticketedSeat();

    CoachData data;
    data.coachName = seat.seatSection();
    if (seat.seatingType() == "1"_L1) {
        data.coachClass = KPublicTransport::VehicleSection::FirstClass;
    } else if (seat.seatingType() == "2"_L1) {
        data.coachClass = KPublicTransport::VehicleSection::SecondClass;
    }
    return data;
}

static QString platformSectionsForCoachData(const KPublicTransport::Stopover &stop, const CoachData &coach)
{
    if (!coach.coachName.isEmpty()) {
        return KPublicTransport::PlatformLayout::sectionsForVehicleSection(stop, coach.coachName);
    }
    if (coach.coachClass != KPublicTransport::VehicleSection::UnknownClass) {
        return KPublicTransport::PlatformLayout::sectionsForClass(stop, coach.coachClass);
    }
    return KPublicTransport::PlatformLayout::sectionsForVehicle(stop);
}

static void mapArgumentsForPt(QJSValue &args, QLatin1StringView prefix, const KPublicTransport::Stopover &stop, const CoachData &coach)
{
    const auto platformName = stop.hasExpectedPlatform() ? stop.expectedPlatform() : stop.scheduledPlatform();
    if (!platformName.isEmpty()) {
        const auto sections = platformSectionsForCoachData(stop, coach);
        args.setProperty(prefix + "PlatformName"_L1, sections.isEmpty() ? platformName : (platformName + ' '_L1 + sections));
    }

    if (stop.route().line().mode() != KPublicTransport::Line::Unknown) {
        args.setProperty(prefix + "PlatformMode"_L1, PublicTransport::lineModeToPlatformMode(stop.route().line().mode()));
    }

    const auto ifopt = stop.stopPoint().identifier(u"ifopt"_s);
    if (!ifopt.isEmpty()) {
        args.setProperty(prefix + "PlatformIfopt"_L1, ifopt);
    }
}

static void mapArrivalArgumesForRes(QJSValue &args, const QVariant &res)
{
    if (JsonLd::isA<TrainReservation>(res)) {
        const auto trip = res.value<TrainReservation>().reservationFor().value<TrainTrip>();
        args.setProperty(u"arrivalPlatformName"_s, trip.arrivalPlatform());
        args.setProperty(u"arrivalPlatformMode"_s, KOSMIndoorMap::Platform::Rail);
    } else if (JsonLd::isA<BusReservation>(res)) {
        const auto trip = res.value<BusReservation>().reservationFor().value<BusTrip>();
        args.setProperty(u"arrivalPlatformName"_s, trip.arrivalPlatform());
        args.setProperty(u"arrivalPlatformMode"_s, KOSMIndoorMap::Platform::Bus);
    }
    // TODO there is no arrival gate property (yet)
}

static void mapDepartureArgumentsForRes(QJSValue &args, const QVariant &res)
{
    if (JsonLd::isA<TrainReservation>(res)) {
        const auto trip = res.value<TrainReservation>().reservationFor().value<TrainTrip>();
        args.setProperty(u"departurePlatformName"_s, trip.departurePlatform());
        args.setProperty(u"departurePlatformMode"_s, KOSMIndoorMap::Platform::Rail);
    } else if (JsonLd::isA<BusReservation>(res)) {
        const auto trip = res.value<BusReservation>().reservationFor().value<BusTrip>();
        args.setProperty(u"departurePlatformName"_s, trip.departurePlatform());
        args.setProperty(u"departurePlatformMode"_s, KOSMIndoorMap::Platform::Bus);
    } else if (JsonLd::isA<FlightReservation>(res)) {
        const auto flight = res.value<FlightReservation>().reservationFor().value<Flight>();
        args.setProperty(u"departureGateName"_s, flight.departureGate());
    }
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
    mapArgumentsForLocation(args, LocationUtil::arrivalLocation(res), engine);

    // arrival location
    mapArrivalArgumesForRes(args, res);
    const auto arr = arrival();
    mapArgumentsForPt(args, "arrival"_L1, arr, coachDataForReservation(res));

    // arrival time
    auto arrTime = arr.hasExpectedArrivalTime() ? arr.expectedArrivalTime() : arr.scheduledArrivalTime();
    if (!arrTime.isValid()) {
        arrTime = SortUtil::endDateTime(res);
    }
    args.setProperty(u"beginTime"_s, engine->toScriptValue(arrTime));
    if (arrTime.timeSpec() == Qt::TimeZone) {
        args.setProperty(u"timeZone"_s, QString::fromUtf8(arrTime.timeZone().id()));
    }

    // look for departure for a following transfer
    const auto transfer = m_transferMgr->transfer(m_batchId, Transfer::After);
    if (transfer.state() == Transfer::Selected) {
        const auto dep = PublicTransport::firstTransportSection(transfer.journey()).departure();
        mapArgumentsForPt(args, "departure"_L1, dep, {});
        args.setProperty(u"endTime"_s, engine->toScriptValue(dep.hasExpectedDepartureTime() ? dep.expectedDepartureTime() : dep.scheduledDepartureTime()));
        return args;
    }

    // ... or layover
    const auto nextResId = m_resMgr->nextBatch(m_batchId);
    const auto nextRes = m_resMgr->reservation(nextResId);
    if (!isLayover(res, nextRes)) {
        return args;
    }
    mapDepartureArgumentsForRes(args, nextRes);
    const auto dep = m_liveDataMgr->departure(nextResId);
    mapArgumentsForPt(args, "departure"_L1, dep, coachDataForReservation(nextRes));

    auto depTime = dep.hasExpectedDepartureTime() ? dep.expectedDepartureTime() : dep.scheduledDepartureTime();
    if (!depTime.isValid()) {
        depTime = SortUtil::startDateTime(nextRes);
    }
    args.setProperty(u"endTime"_s, engine->toScriptValue(depTime));

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
    mapArgumentsForLocation(args, LocationUtil::departureLocation(res), engine);

    // departure location
    mapDepartureArgumentsForRes(args, res);
    const auto dep = departure();
    mapArgumentsForPt(args, "departure"_L1, dep, coachDataForReservation(res));

    // departure time
    auto depTime = dep.hasExpectedDepartureTime() ? dep.expectedDepartureTime() : dep.scheduledDepartureTime();
    if (!depTime.isValid()) {
        depTime = SortUtil::startDateTime(res);
    }
    args.setProperty(u"endTime"_s, engine->toScriptValue(depTime));
    if (depTime.timeSpec() == Qt::TimeZone) {
        args.setProperty(u"timeZone"_s, QString::fromUtf8(depTime.timeZone().id()));
    }

    // look for arrival for a preceding transfer
    const auto transfer = m_transferMgr->transfer(m_batchId, Transfer::Before);
    if (transfer.state() == Transfer::Selected) {
        const auto arr = PublicTransport::lastTransportSection(transfer.journey()).arrival();
        mapArgumentsForPt(args, "arrival"_L1, arr, {});
        args.setProperty(u"beginTime"_s, engine->toScriptValue(arr.hasExpectedArrivalTime() ? arr.expectedArrivalTime() : arr.scheduledArrivalTime()));
        return args;
    }

    // ... or layover
    const auto prevResId = m_resMgr->previousBatch(m_batchId);
    const auto prevRes = m_resMgr->reservation(prevResId);
    if (!isLayover(prevRes, res)) {
        return args;
    }
    mapArrivalArgumesForRes(args, prevRes);
    const auto arr = m_liveDataMgr->arrival(prevResId);
    mapArgumentsForPt(args, "arrival"_L1, arr, coachDataForReservation(prevRes));

    auto arrTime = arr.hasExpectedArrivalTime() ? arr.expectedArrivalTime() : arr.scheduledArrivalTime();
    if (!arrTime.isValid()) {
        arrTime = SortUtil::endDateTime(prevRes);
    }
    args.setProperty(u"beginTime"_s, engine->toScriptValue(arrTime));

    return args;
}

QJSValue TimelineDelegateController::mapArguments() const
{
    const auto engine = qjsEngine(this);
    if (!engine || !m_resMgr || m_batchId.isEmpty() || !m_transferMgr || !m_liveDataMgr) {
        return {};
    }

    const auto res = m_resMgr->reservation(m_batchId);
    if (LocationUtil::isLocationChange(res)) {
        return {};
    }

    auto args = engine->newObject();
    mapArgumentsForLocation(args, LocationUtil::location(res), engine);

    // determine time on site, considering the following sources:
    // (1) the full days res is covering
    // (2) arrival time of a preceding location change, departure time of a following location change
    // (3) arrival time of a preceding transfer, departure time of a following transfer

    auto beginDt = SortUtil::startDateTime(res);
    beginDt.setTime({});

    for (auto prevResId = m_resMgr->previousBatch(m_batchId); !prevResId.isEmpty(); prevResId = m_resMgr->previousBatch(prevResId)) {
        const auto prevRes = m_resMgr->reservation(prevResId);
        if (LocationUtil::isLocationChange(prevRes)) {
            beginDt = std::max(SortUtil::endDateTime(prevRes), beginDt);
            break;
        }
    }

    auto transfer = m_transferMgr->transfer(m_batchId, Transfer::Before);
    if (transfer.state() == Transfer::Selected) {
        const auto arr = PublicTransport::lastTransportSection(transfer.journey()).arrival();
        mapArgumentsForPt(args, "arrival"_L1, arr, {});
        beginDt = std::max(arr.hasExpectedArrivalTime() ? arr.expectedArrivalTime() : arr.scheduledArrivalTime(), beginDt);
    }

    args.setProperty(u"beginTime"_s, engine->toScriptValue(beginDt));

    auto endDt = SortUtil::endDateTime(res);
    if (endDt.isValid()) {
        endDt = endDt.addDays(1);
        endDt.setTime({});
    }

    transfer = m_transferMgr->transfer(m_batchId, Transfer::After);
    if (transfer.state() == Transfer::Selected) {
        const auto dep = PublicTransport::firstTransportSection(transfer.journey()).departure();
        mapArgumentsForPt(args, "departure"_L1, dep, {});
        const auto depDt = dep.hasExpectedDepartureTime() ? dep.expectedDepartureTime() : dep.scheduledDepartureTime();
        endDt = endDt.isValid() ? std::min(endDt, depDt) : depDt;
    }

    // search the first location change after the end (which might not always be the next element)
    const auto dt = SortUtil::endDateTime(res);
    auto nextResId = m_resMgr->nextBatch(m_batchId);
    auto nextRes = m_resMgr->reservation(nextResId);
    while (dt.isValid() && !nextResId.isEmpty() && SortUtil::startDateTime(nextRes).date() <= dt.date()) {
        if (LocationUtil::isLocationChange(nextRes)) {
            const auto depDt = SortUtil::startDateTime(nextRes);
            if (depDt.isValid() && depDt >= SortUtil::endDateTime((res))) {
                endDt = endDt.isValid() ? std::min(depDt, endDt) : depDt;
                break;
            }
        }

        nextResId = m_resMgr->nextBatch(nextResId);
        nextRes = m_resMgr->reservation(nextResId);
    }

    if (endDt.isValid()) {
        args.setProperty(u"endTime"_s, engine->toScriptValue(endDt));
    }

    return args;
}

void TimelineDelegateController::addToCalendar(KCalendarCore::Calendar *cal)
{
    const auto resIds = m_resMgr->reservationsForBatch(m_batchId);
    if (resIds.isEmpty() || !cal) {
        return;
    }
    QList<QVariant> reservations;
    reservations.reserve(resIds.size());
    for (const auto &resId : resIds) {
        reservations.push_back(m_resMgr->reservation(resId));
    }

    const auto existingEvents = CalendarHandler::findEvents(cal, reservations.at(0));
    KCalendarCore::Event::Ptr event;
    if (existingEvents.size() == 1) {
        event = existingEvents.at(0);
        event->startUpdates();
    } else {
        event = KCalendarCore::Event::Ptr(new KCalendarCore::Event);
    }

    CalendarHandler::fillEvent(reservations, event);
    CalendarHelper::fillPreTransfer(event, m_transferMgr->transfer(m_batchId, Transfer::Before));

    if (existingEvents.size() == 1) {
        event->endUpdates();
    } else {
        cal->addEvent(event);
    }
}

static void applyVehicleLayout(KPublicTransport::Stopover &stop, const KPublicTransport::Stopover &layout)
{
    if (PublicTransport::isSameStopoverForLayout(stop, layout)) {
        stop = KPublicTransport::Stopover::merge(stop, layout);
    }
}

void TimelineDelegateController::setVehicleLayout(const KPublicTransport::Stopover &stopover, bool arrival)
{
    auto jny = journey();
    if (!arrival) {
        auto dep = jny.departure();
        applyVehicleLayout(dep, stopover);
        jny.setDeparture(dep);
        m_liveDataMgr->applyJourney(m_batchId, jny);
    } else {
        auto arr = jny.arrival();
        applyVehicleLayout(arr, stopover);
        jny.setArrival(arr);
        m_liveDataMgr->applyJourney(m_batchId, jny);
    }

    Q_EMIT layoutChanged();
}

QString TimelineDelegateController::departurePlatformSections() const
{
    if (!m_resMgr) {
        return {};
    }
    const auto res = m_resMgr->reservation(m_batchId);
    return platformSectionsForCoachData(departure(), coachDataForReservation(res));
}

QString TimelineDelegateController::arrivalPlatformSections() const
{
    if (!m_resMgr) {
        return {};
    }
    const auto res = m_resMgr->reservation(m_batchId);
    return platformSectionsForCoachData(arrival(), coachDataForReservation(res));
}

QStringList TimelineDelegateController::documentIds() const
{
    if (!m_resMgr || !m_documentMgr || m_batchId.isEmpty()) {
        return {};
    }

    QStringList result;
    const auto resIds = m_resMgr->reservationsForBatch(m_batchId);
    for (const auto &resId : resIds) {
        const auto res = m_resMgr->reservation(resId);
        const auto docIds = DocumentUtil::documentIds(res);
        for (const auto &docId : docIds) {
            const auto id = docId.toString();
            if (!id.isEmpty() && m_documentMgr->hasDocument(id)) {
                result.push_back(id);
            }
        }
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

QVariant TimelineDelegateController::reservation() const
{
    if (!m_resMgr || m_batchId.isEmpty()) {
        return {};
    }

    return m_resMgr->reservation(m_batchId);
}

#include "moc_timelinedelegatecontroller.cpp"
