/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "statisticsmodel.h"
#include "livedatamanager.h"
#include "localizer.h"
#include "locationhelper.h"
#include "reservationmanager.h"
#include "transfermanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>

#include <KPublicTransport/Journey>
#include <KPublicTransport/RentalVehicle>

#include <KCountry>
#include <KLocalizedString>

#include <QDebug>

#include <algorithm>

using namespace KItinerary;

StatisticsItem::StatisticsItem() = default;

StatisticsItem::StatisticsItem(const QString &label, const QString &value, StatisticsItem::Trend trend, bool hasData)
    : m_label(label)
    , m_value(value)
    , m_trend(trend)
    , m_hasData(hasData)
{
}

StatisticsItem::~StatisticsItem() = default;

StatisticsModel::StatisticsModel(QObject *parent)
    : QObject(parent)
{
    connect(this, &StatisticsModel::setupChanged, this, &StatisticsModel::recompute);
    recompute();
}

StatisticsModel::~StatisticsModel() = default;

ReservationManager *StatisticsModel::reservationManager() const
{
    return m_resMgr;
}

void StatisticsModel::setReservationManager(ReservationManager *resMgr)
{
    if (m_resMgr == resMgr) {
        return;
    }
    m_resMgr = resMgr;
    connect(m_resMgr, &ReservationManager::batchAdded, this, &StatisticsModel::recompute);
    Q_EMIT setupChanged();
}

TripGroupManager *StatisticsModel::tripGroupManager() const
{
    return m_tripGroupMgr;
}

void StatisticsModel::setTripGroupManager(TripGroupManager *tripGroupMgr)
{
    if (m_tripGroupMgr == tripGroupMgr) {
        return;
    }
    m_tripGroupMgr = tripGroupMgr;
    connect(m_tripGroupMgr, &TripGroupManager::tripGroupAdded, this, &StatisticsModel::recompute);
    Q_EMIT setupChanged();
}

void StatisticsModel::setTransferManager(TransferManager *transferMgr)
{
    if (m_transferMgr == transferMgr) {
        return;
    }
    m_transferMgr = transferMgr;
    connect(m_transferMgr, &TransferManager::transferAdded, this, &StatisticsModel::recompute);
    Q_EMIT setupChanged();
}

void StatisticsModel::setTimeRange(const QDate &begin, const QDate &end)
{
    if (m_begin == begin && end == m_end) {
        return;
    }

    m_begin = begin;
    m_end = end;
    recompute();
}

StatisticsItem StatisticsModel::totalCount() const
{
    return StatisticsItem(i18n("Trips"), QLocale().toString(m_tripGroupCount), trend(m_tripGroupCount, m_prevTripGroupCount));
}

StatisticsItem StatisticsModel::totalDistance() const
{
    return StatisticsItem(i18n("Distance"), Localizer::formatDistance(m_statData[Total][Distance]), trend(Total, Distance));
}

StatisticsItem StatisticsModel::totalNights() const
{
    return StatisticsItem(i18n("Hotel nights"), QLocale().toString(m_hotelCount), trend(m_hotelCount, m_prevHotelCount));
}

StatisticsItem StatisticsModel::totalCO2() const
{
    return StatisticsItem(i18n("CO₂"), Localizer::formatWeight(m_statData[Total][CO2]), trend(Total, CO2));
}

StatisticsItem StatisticsModel::visitedCountries() const
{
    QStringList l;
    l.reserve(m_countries.size());
    std::transform(m_countries.begin(), m_countries.end(), std::back_inserter(l), [](const auto &iso) {
        return iso;
    });
    std::sort(l.begin(), l.end(), [](const auto &lhs, const auto &rhs) {
        return KCountry::fromAlpha2(lhs).name().localeAwareCompare(KCountry::fromAlpha2(rhs).name()) < 0;
    });
    return StatisticsItem(i18n("Visited countries"), l.join(QLatin1Char(' ')), StatisticsItem::TrendUnknown);
}

StatisticsItem StatisticsModel::flightCount() const
{
    return StatisticsItem(i18n("Flights"), QLocale().toString(m_statData[Flight][TripCount]), trend(Flight, TripCount), m_hasData[Flight]);
}

StatisticsItem StatisticsModel::flightDistance() const
{
    return StatisticsItem(i18n("Distance"), Localizer::formatDistance(m_statData[Flight][Distance]), trend(Flight, Distance), m_hasData[Flight]);
}

StatisticsItem StatisticsModel::flightCO2() const
{
    return StatisticsItem(i18n("CO₂"), Localizer::formatWeight(m_statData[Flight][CO2]), trend(Flight, CO2), m_hasData[Flight]);
}

StatisticsItem StatisticsModel::trainCount() const
{
    return StatisticsItem(i18n("Train rides"), QLocale().toString(m_statData[Train][TripCount]), trend(Train, TripCount), m_hasData[Train]);
}

StatisticsItem StatisticsModel::trainDistance() const
{
    return StatisticsItem(i18n("Distance"), Localizer::formatDistance(m_statData[Train][Distance]), trend(Train, Distance), m_hasData[Train]);
}

StatisticsItem StatisticsModel::trainCO2() const
{
    return StatisticsItem(i18n("CO₂"), Localizer::formatWeight(m_statData[Train][CO2]), trend(Train, CO2), m_hasData[Train]);
}

StatisticsItem StatisticsModel::busCount() const
{
    return StatisticsItem(i18n("Bus rides"), QLocale().toString(m_statData[Bus][TripCount]), trend(Bus, TripCount), m_hasData[Bus]);
}

StatisticsItem StatisticsModel::busDistance() const
{
    return StatisticsItem(i18n("Distance"), Localizer::formatDistance(m_statData[Bus][Distance]), trend(Bus, Distance), m_hasData[Bus]);
}

StatisticsItem StatisticsModel::busCO2() const
{
    return StatisticsItem(i18n("CO₂"), Localizer::formatWeight(m_statData[Bus][CO2]), trend(Bus, CO2), m_hasData[Bus]);
}

StatisticsItem StatisticsModel::carCount() const
{
    return StatisticsItem(i18n("Car rides"), QLocale().toString(m_statData[Car][TripCount]), trend(Car, TripCount), m_hasData[Car]);
}

StatisticsItem StatisticsModel::carDistance() const
{
    return StatisticsItem(i18n("Distance"), Localizer::formatDistance(m_statData[Car][Distance]), trend(Car, Distance), m_hasData[Car]);
}

StatisticsItem StatisticsModel::carCO2() const
{
    return StatisticsItem(i18n("CO₂"), Localizer::formatWeight(m_statData[Car][CO2]), trend(Car, CO2), m_hasData[Car]);
}

StatisticsItem StatisticsModel::boatCount() const
{
    return StatisticsItem(i18n("Boat trips"), QLocale().toString(m_statData[Boat][TripCount]), trend(Boat, TripCount), m_hasData[Boat]);
}

StatisticsItem StatisticsModel::boatDistance() const
{
    return StatisticsItem(i18n("Distance"), Localizer::formatDistance(m_statData[Boat][Distance]), trend(Boat, Distance), m_hasData[Boat]);
}

StatisticsItem StatisticsModel::boatCO2() const
{
    return StatisticsItem(i18n("CO₂"), Localizer::formatWeight(m_statData[Boat][CO2]), trend(Boat, CO2), m_hasData[Boat]);
}

StatisticsModel::AggregateType StatisticsModel::typeForReservation(const QVariant &res)
{
    if (JsonLd::isA<FlightReservation>(res)) {
        return Flight;
    }
    if (JsonLd::isA<TrainReservation>(res)) {
        return Train;
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return Bus;
    }
    if (JsonLd::isA<BoatReservation>(res)) {
        return Boat;
    }
    return Car;
}

// from https://en.wikipedia.org/wiki/Environmental_impact_of_transport
static const int emissionPerKm[] = {
    0,
    285, // flight
    14, // train
    68, // bus
    158, // car
    113, // ferry
};

double StatisticsModel::co2emission(StatisticsModel::AggregateType type, double distance)
{
    return distance * emissionPerKm[type];
}

double StatisticsModel::co2Emission(const QVariant &res)
{
    return co2emission(typeForReservation(res), LocationHelper::distance(res) / 1000.0);
}

void StatisticsModel::computeStats(const QString &resId, const QVariant &res, int (&statData)[AGGREGATE_TYPE_COUNT][STAT_TYPE_COUNT])
{
    const auto type = typeForReservation(res);
    double dist = LocationHelper::distance(res);
    double co2 = co2emission(type, dist / 1000.0);

    // live data is more accurate when present
    if (const auto jny = m_transferMgr->liveDataManager()->journey(resId); jny.mode() != KPublicTransport::JourneySection::Invalid) {
        dist = std::max<double>(dist, jny.distance()); // we cannot possibly get shorter than the direct distance
        co2 = jny.co2Emission();
    }

    statData[type][TripCount]++;
    statData[type][Distance] += dist;
    statData[type][CO2] += co2;

    statData[Total][TripCount]++;
    statData[Total][Distance] += dist;
    statData[Total][CO2] += co2;
}

void StatisticsModel::computeStats(const KPublicTransport::Journey &journey, int (&statData)[AGGREGATE_TYPE_COUNT][STAT_TYPE_COUNT])
{
    struct {
        KPublicTransport::Line::Mode mode;
        StatisticsModel::AggregateType type;
    } static constexpr const lineModeMap[] = {
        { KPublicTransport::Line::Air, StatisticsModel::Flight },
        { KPublicTransport::Line::Boat, StatisticsModel::Boat },
        { KPublicTransport::Line::Bus, StatisticsModel::Bus },
        { KPublicTransport::Line::Coach, StatisticsModel::Bus },
        { KPublicTransport::Line::Ferry, StatisticsModel::Boat },
        { KPublicTransport::Line::Funicular, StatisticsModel::Train },
        { KPublicTransport::Line::LocalTrain, StatisticsModel::Train },
        { KPublicTransport::Line::LongDistanceTrain, StatisticsModel::Train },
        { KPublicTransport::Line::Metro, StatisticsModel::Train },
        { KPublicTransport::Line::RailShuttle, StatisticsModel::Train },
        { KPublicTransport::Line::Shuttle, StatisticsModel::Bus },
        { KPublicTransport::Line::Taxi, StatisticsModel::Car },
        { KPublicTransport::Line::Train, StatisticsModel::Train },
        { KPublicTransport::Line::Tramway, StatisticsModel::Train },
        { KPublicTransport::Line::RideShare, StatisticsModel::Car },
        // { KPublicTransport::Line::AerialLift, StatisticsModel::Train }, ### where to map this to?
    };


    for (const auto &jny :journey.sections()) {
        StatisticsModel::AggregateType type = StatisticsModel::Total;
        switch (jny.mode()) {
            case KPublicTransport::JourneySection::Invalid:
            case KPublicTransport::JourneySection::Walking:
            case KPublicTransport::JourneySection::Transfer:
            case KPublicTransport::JourneySection::Waiting:
                break;
            case KPublicTransport::JourneySection::PublicTransport:
                for (const auto &m :lineModeMap) {
                    if (m.mode == jny.route().line().mode()) {
                        type = m.type;
                    }
                }
                break;
            case KPublicTransport::JourneySection::IndividualTransport:
                if (jny.individualTransport().mode() == KPublicTransport::IndividualTransport::Car) {
                    type = StatisticsModel::Car;
                }
                break;
            case KPublicTransport::JourneySection::RentedVehicle:
                if (jny.rentalVehicle().type() == KPublicTransport::RentalVehicle::Car) {
                    type = StatisticsModel::Car;
                }
                break;
        }
        if (type == StatisticsModel::Total) {
            continue;
        }

        statData[type][TripCount]++;
        statData[type][Distance] += jny.distance();
        statData[type][CO2] += jny.co2Emission();

        statData[Total][TripCount]++;
        statData[Total][Distance] += jny.distance();
        statData[Total][CO2] += jny.co2Emission();
    }
}

void StatisticsModel::recompute()
{
    memset(m_statData, 0, AGGREGATE_TYPE_COUNT * STAT_TYPE_COUNT * sizeof(int));
    memset(m_prevStatData, 0, AGGREGATE_TYPE_COUNT * STAT_TYPE_COUNT * sizeof(int));
    memset(m_hasData, 0, AGGREGATE_TYPE_COUNT * sizeof(bool));
    m_hasData[Total] = true;
    m_hotelCount = 0;
    m_prevHotelCount = 0;
    m_countries.clear();

    if (!m_resMgr || !m_tripGroupMgr || !m_transferMgr) {
        return;
    }

    QDate prevStart;
    if (m_begin.isValid() && m_end.isValid()) {
        prevStart = m_begin.addDays(m_end.daysTo(m_begin));
    }

    QSet<QString> tripGroups, prevTripGroups;

    const auto &batches = m_resMgr->batches();
    for (const auto &batchId : batches) {
        const auto res = m_resMgr->reservation(batchId);
        if (LocationUtil::isLocationChange(res)) {
            m_hasData[typeForReservation(res)] = true;
        }
        const auto dt = SortUtil::startDateTime(res);

        bool isPrev = false;
        if (m_end.isValid() && dt.date() > m_end) {
            continue;
        }
        if (prevStart.isValid()) {
            if (dt.date() < prevStart) {
                continue;
            }
            isPrev = dt.date() < m_begin;
        }

        // don't count canceled reservations
        if (JsonLd::canConvert<Reservation>(res) && JsonLd::convert<Reservation>(res).reservationStatus() == Reservation::ReservationCancelled) {
            continue;
        }

        if (LocationUtil::isLocationChange(res)) {
            computeStats(batchId, res, isPrev ? m_prevStatData : m_statData);
        } else if (JsonLd::isA<LodgingReservation>(res)) {
            const auto hotel = res.value<LodgingReservation>();
            if (isPrev) {
                m_prevHotelCount += hotel.checkinTime().daysTo(hotel.checkoutTime());
            } else {
                m_hotelCount += hotel.checkinTime().daysTo(hotel.checkoutTime());
            }
        }

        for (const auto alignment : { Transfer::Before, Transfer::After}) {
            if (const auto t = m_transferMgr->transfer(batchId, alignment); t.state() == Transfer::Selected) {
                computeStats(t.journey(), isPrev ? m_prevStatData : m_statData);
            }
        }

        const auto tgId = m_tripGroupMgr->tripGroupIdForReservation(batchId);
        if (isRelevantTripGroup(tgId)) {
            isPrev ? prevTripGroups.insert(tgId) : tripGroups.insert(tgId);
        }

        if (!isPrev) {
            auto c = LocationHelper::departureCountry(res);
            if (!c.isEmpty()) {
                m_countries.insert(c);
            }
            c = LocationHelper::destinationCountry(res);
            if (!c.isEmpty()) {
                m_countries.insert(c);
            }
        }
    }

    m_tripGroupCount = tripGroups.size();
    m_prevTripGroupCount = prevTripGroups.size();

    Q_EMIT changed();
}

bool StatisticsModel::isRelevantTripGroup(const QString &tgId) const
{
    const auto tg = m_tripGroupMgr->tripGroup(tgId);
    const auto elems = tg.elements();
    return std::any_of(elems.begin(), elems.end(), [this](const QString &elem) {
        const auto res = m_resMgr->reservation(elem);
        return LocationUtil::isLocationChange(res);
    });
}

StatisticsItem::Trend StatisticsModel::trend(int current, int prev) const
{
    if (!m_begin.isValid() || !m_end.isValid()) {
        return StatisticsItem::TrendUnknown;
    }

    return current < prev ? StatisticsItem::TrendDown : current > prev ? StatisticsItem::TrendUp : StatisticsItem::TrendUnchanged;
}

StatisticsItem::Trend StatisticsModel::trend(StatisticsModel::AggregateType type, StatisticsModel::StatType stat) const
{
    return trend(m_statData[type][stat], m_prevStatData[type][stat]);
}

#include "moc_statisticsmodel.cpp"
