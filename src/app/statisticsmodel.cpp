/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "statisticsmodel.h"
#include "locationhelper.h"
#include "localizer.h"
#include "reservationmanager.h"
#include "tripgroupmanager.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>

#include <KLocalizedString>

#include <QDebug>

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

ReservationManager* StatisticsModel::reservationManager() const
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

TripGroupManager* StatisticsModel::tripGroupManager() const
{
    return m_tripGroupMgr;
}

void StatisticsModel::setTripGroupManager(TripGroupManager* tripGroupMgr)
{
    if (m_tripGroupMgr == tripGroupMgr) {
        return;
    }
    m_tripGroupMgr = tripGroupMgr;
    connect(m_tripGroupMgr, &TripGroupManager::tripGroupAdded, this, &StatisticsModel::recompute);
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

static QString formatCo2(int amount)
{
    if (amount >= 10000) {
        // no decimals for large values
        return i18n("%1 kg", amount / 1000);
    }
    return ki18n("%1 kg").subs(amount / 1000.0, 0, 'g', 2).toString();
}

StatisticsItem StatisticsModel::totalCount() const
{
    return StatisticsItem(i18n("Trips"), QLocale().toString(m_tripGroupCount), trend(m_tripGroupCount, m_prevTripGroupCount));
}

StatisticsItem StatisticsModel::totalDistance() const
{
    return StatisticsItem(i18n("Distance"), i18n("%1 km", m_statData[Total][Distance] / 1000), trend(Total, Distance));
}

StatisticsItem StatisticsModel::totalNights() const
{
    return StatisticsItem(i18n("Hotel nights"), QLocale().toString(m_hotelCount), trend(m_hotelCount, m_prevHotelCount));
}

StatisticsItem StatisticsModel::totalCO2() const
{
    return StatisticsItem(i18n("CO₂"), formatCo2(m_statData[Total][CO2]), trend(Total, CO2));
}

StatisticsItem StatisticsModel::visitedCountries() const
{
    QStringList l;
    l.reserve(m_countries.size());
    std::transform(m_countries.begin(), m_countries.end(), std::back_inserter(l), [](const auto &iso) {
        return iso;
    });
    return StatisticsItem(i18n("Visited countries"), l.join(QLatin1Char(' ')), StatisticsItem::TrendUnknown);
}

StatisticsItem StatisticsModel::flightCount() const
{
    return StatisticsItem(i18n("Flights"), QLocale().toString(m_statData[Flight][TripCount]), trend(Flight, TripCount), m_hasData[Flight]);
}

StatisticsItem StatisticsModel::flightDistance() const
{
    return StatisticsItem(i18n("Distance"), i18n("%1 km", m_statData[Flight][Distance] / 1000), trend(Flight, Distance), m_hasData[Flight]);
}

StatisticsItem StatisticsModel::flightCO2() const
{
    return StatisticsItem(i18n("CO₂"), formatCo2(m_statData[Flight][CO2]), trend(Flight, CO2), m_hasData[Flight]);
}

StatisticsItem StatisticsModel::trainCount() const
{
    return StatisticsItem(i18n("Train rides"), QLocale().toString(m_statData[Train][TripCount]), trend(Train, TripCount), m_hasData[Train]);
}

StatisticsItem StatisticsModel::trainDistance() const
{
    return StatisticsItem(i18n("Distance"), i18n("%1 km", m_statData[Train][Distance] / 1000), trend(Train, Distance), m_hasData[Train]);
}

StatisticsItem StatisticsModel::trainCO2() const
{
    return StatisticsItem(i18n("CO₂"), formatCo2(m_statData[Train][CO2]), trend(Train, CO2), m_hasData[Train]);
}

StatisticsItem StatisticsModel::busCount() const
{
    return StatisticsItem(i18n("Bus rides"), QLocale().toString(m_statData[Bus][TripCount]), trend(Bus, TripCount), m_hasData[Bus]);
}

StatisticsItem StatisticsModel::busDistance() const
{
    return StatisticsItem(i18n("Distance"), i18n("%1 km", m_statData[Bus][Distance] / 1000), trend(Bus, Distance), m_hasData[Bus]);
}

StatisticsItem StatisticsModel::busCO2() const
{
    return StatisticsItem(i18n("CO₂"), formatCo2(m_statData[Bus][CO2]), trend(Bus, CO2), m_hasData[Bus]);
}

StatisticsItem StatisticsModel::carCount() const
{
    return StatisticsItem(i18n("Car rides"), QLocale().toString(m_statData[Car][TripCount]), trend(Car, TripCount), m_hasData[Car]);
}

StatisticsItem StatisticsModel::carDistance() const
{
    return StatisticsItem(i18n("Distance"), i18n("%1 km", m_statData[Car][Distance] / 1000), trend(Car, Distance), m_hasData[Car]);
}

StatisticsItem StatisticsModel::carCO2() const
{
    return StatisticsItem(i18n("CO₂"), formatCo2(m_statData[Car][CO2]), trend(Car, CO2), m_hasData[Car]);
}

StatisticsItem StatisticsModel::boatCount() const
{
    return StatisticsItem(i18n("Boat trips"), QLocale().toString(m_statData[Boat][TripCount]), trend(Boat, TripCount), m_hasData[Boat]);
}

StatisticsItem StatisticsModel::boatDistance() const
{
    return StatisticsItem(i18n("Distance"), i18n("%1 km", m_statData[Boat][Distance] / 1000), trend(Boat, Distance), m_hasData[Boat]);
}

StatisticsItem StatisticsModel::boatCO2() const
{
    return StatisticsItem(i18n("CO₂"), formatCo2(m_statData[Boat][CO2]), trend(Boat, CO2), m_hasData[Boat]);
}

StatisticsModel::AggregateType StatisticsModel::typeForReservation(const QVariant &res) const
{
    if (JsonLd::isA<FlightReservation>(res)) {
        return Flight;
    } else if (JsonLd::isA<TrainReservation>(res)) {
        return Train;
    } else if (JsonLd::isA<BusReservation>(res)) {
        return Bus;
    } else if (JsonLd::isA<BoatReservation>(res)) {
        return Boat;
    }
    return Car;
}

static int distance(const QVariant &res)
{
    const auto dep = LocationUtil::departureLocation(res);
    const auto arr = LocationUtil::arrivalLocation(res);
    if (dep.isNull() || arr.isNull()) {
        return 0;
    }
    const auto depGeo = LocationUtil::geo(dep);
    const auto arrGeo = LocationUtil::geo(arr);
    if (!depGeo.isValid() || !arrGeo.isValid()) {
        return 0;
    }
    return std::max(0, LocationUtil::distance(depGeo, arrGeo));
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

int StatisticsModel::co2emission(StatisticsModel::AggregateType type, int distance) const
{
    return distance * emissionPerKm[type];
}

void StatisticsModel::computeStats(const QVariant& res, int (&statData)[AGGREGATE_TYPE_COUNT][STAT_TYPE_COUNT])
{
    const auto type = typeForReservation(res);
    const auto dist = distance(res);
    const auto co2 = co2emission(type, dist / 1000);

    statData[type][TripCount]++;
    statData[type][Distance] += dist;
    statData[type][CO2] += co2;

    statData[Total][TripCount]++;
    statData[Total][Distance] += dist;
    statData[Total][CO2] += co2;
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

    if (!m_resMgr || !m_tripGroupMgr) {
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
            computeStats(res, isPrev ? m_prevStatData : m_statData);
        } else if (JsonLd::isA<LodgingReservation>(res)) {
            const auto hotel = res.value<LodgingReservation>();
            if (isPrev) {
                m_prevHotelCount += hotel.checkinTime().daysTo(hotel.checkoutTime());
            } else {
                m_hotelCount += hotel.checkinTime().daysTo(hotel.checkoutTime());
            }
        }

        const auto tgId = m_tripGroupMgr->tripGroupIdForReservation(batchId);
        if (!tgId.isEmpty()) {
            isPrev ? prevTripGroups.insert(tgId) : tripGroups.insert(tgId);
        }

        if (!isPrev) {
            auto c = LocationHelper::departureCountry(res);
            if (!c.isEmpty()) m_countries.insert(c);
            c = LocationHelper::destinationCountry(res);
            if (!c.isEmpty()) m_countries.insert(c);
        }
    }

    m_tripGroupCount = tripGroups.size();
    m_prevTripGroupCount = prevTripGroups.size();

    Q_EMIT changed();
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
