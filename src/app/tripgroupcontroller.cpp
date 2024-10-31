/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tripgroupcontroller.h"

#include "locationinformation.h"
#include "reservationmanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/Place>
#include <KItinerary/SortUtil>

#include <KCountry>

#include <QDateTime>
#include <QDebug>

using namespace KItinerary;

static GeoCoordinates geoCoordinate(const QVariant &res)
{
    if (LocationUtil::isLocationChange(res)) {
        return LocationUtil::geo(LocationUtil::arrivalLocation(res));
    }
    return LocationUtil::geo(LocationUtil::location(res));
}

TripGroupController::TripGroupController(QObject *parent)
    : QObject(parent)
{
    connect(this, &TripGroupController::setupChanged, this, &TripGroupController::adjacencyChanged);
    connect(this, &TripGroupController::tripGroupChanged, this, &TripGroupController::adjacencyChanged);
    connect(this, &TripGroupController::setupChanged, this, &TripGroupController::weatherForecastChanged);
    connect(this, &TripGroupController::tripGroupChanged, this, &TripGroupController::weatherForecastChanged);
    connect(this, &TripGroupController::setupChanged, this, &TripGroupController::locationInfoChanged);
    connect(this, &TripGroupController::tripGroupChanged, this, &TripGroupController::locationInfoChanged);
    connect(this, &TripGroupController::tripGroupChanged, this, &TripGroupController::tripGroupContentChanged);
}

TripGroupController::~TripGroupController() = default;

void TripGroupController::setTripGroupModel(TripGroupModel *tgModel)
{
    if (m_tripGroupModel == tgModel) {
        return;
    }

    m_tripGroupModel = tgModel;

    connect(tgModel, &TripGroupModel::rowsInserted, this, &TripGroupController::adjacencyChanged);
    connect(tgModel, &TripGroupModel::rowsRemoved, this, &TripGroupController::adjacencyChanged);

    connect(tgModel->tripGroupManager(), &TripGroupManager::tripGroupChanged, this, [this](const auto &id) {
        if (id == m_tgId) {
            Q_EMIT weatherForecastChanged();
            Q_EMIT locationInfoChanged();
            Q_EMIT adjacencyChanged();
            Q_EMIT tripGroupContentChanged();
        }
    });

    Q_EMIT setupChanged();
}

void TripGroupController::setWeatherForecastManager(WeatherForecastManager *weatherMgr)
{
    if (m_weatherMgr == weatherMgr) {
        return;
    }

    m_weatherMgr = weatherMgr;
    connect(weatherMgr, &WeatherForecastManager::forecastUpdated, this, &TripGroupController::weatherForecastChanged);

    Q_EMIT setupChanged();
}

WeatherForecast TripGroupController::weatherForecast() const
{
    if (!m_tripGroupModel || !m_weatherMgr || !m_weatherMgr->allowNetworkAccess()) {
        return {};
    }

    const auto group = m_tripGroupModel->tripGroupManager()->tripGroup(m_tgId);
    const auto elems = group.elements();
    if (elems.empty()) {
        return {};
    }

    QVariant startRes;
    QDateTime lastEndTime = group.beginDateTime();
    WeatherForecast fc;

    // home weather before departure
    if (const auto res = m_tripGroupModel->tripGroupManager()->reservationManager()->reservation(elems.front()); LocationUtil::isLocationChange(res)) {
        const auto depTime = SortUtil::startDateTime(res);
        const auto startGeo = LocationUtil::geo(LocationUtil::departureLocation(res));
        if (startGeo.isValid() && depTime.isValid()) {
            const auto newFc = m_weatherMgr->forecast(startGeo.latitude(), startGeo.longitude(), depTime.date().startOfDay(), depTime);
            if (!fc.isValid() && newFc.isValid()) {
                fc.setDateTime(depTime.date().startOfDay());
                fc.setRange(std::numeric_limits<int>::max());
            }
            fc.merge(newFc);
        }
    }

    for (const auto &resId : elems) {
        const auto res = m_tripGroupModel->tripGroupManager()->reservationManager()->reservation(resId);
        const auto newGeo = geoCoordinate(res);
        if (!newGeo.isValid()) {
            continue;
        }
        if (startRes.isValid()) {
            const auto geo = geoCoordinate(startRes);
            const auto startDt = LocationUtil::isLocationChange(startRes) ? SortUtil::endDateTime(startRes) : lastEndTime;
            const auto endDt = SortUtil::startDateTime(res);
            if (geo.isValid() && startDt.isValid() && endDt.isValid()) {
                const auto newFc = m_weatherMgr->forecast(geo.latitude(), geo.longitude(), startDt, endDt);
                if (!fc.isValid() && newFc.isValid()) {
                    fc.setDateTime(startDt);
                    fc.setRange(std::numeric_limits<int>::max()); // always consider the sub-range forecast during merging
                }
                fc.merge(newFc);
            }
            lastEndTime = endDt;
        }
        startRes = res;
    }

    // home weather after arrival
    if (const auto res = m_tripGroupModel->tripGroupManager()->reservationManager()->reservation(elems.back()); LocationUtil::isLocationChange(res)) {
        const auto arrTime = SortUtil::endDateTime(res);
        const auto endGeo = LocationUtil::geo(LocationUtil::arrivalLocation(res));
        if (endGeo.isValid() && arrTime.isValid()) {
            const auto newFc = m_weatherMgr->forecast(endGeo.latitude(), endGeo.longitude(), arrTime, arrTime.date().endOfDay());
            if (!fc.isValid() && newFc.isValid()) {
                fc.setDateTime(arrTime);
                fc.setRange(std::numeric_limits<int>::max());
            }
            fc.merge(newFc);
        }
    }

    return fc;
}

QVariantList TripGroupController::locationInformation() const
{
    if (!m_tripGroupModel) {
        return {};
    }

    const auto group = m_tripGroupModel->tripGroupManager()->tripGroup(m_tgId);
    const auto elems = group.elements();
    QVariantList l;

    for (const auto &resId : elems) {
        const auto res = m_tripGroupModel->tripGroupManager()->reservationManager()->reservation(resId);
        if (!LocationUtil::isLocationChange(res)) {
            continue;
        }

        const auto destCountry = LocationUtil::address(LocationUtil::arrivalLocation(res)).addressCountry();
        if (destCountry.isEmpty() || destCountry == m_homeCountry) {
            continue;
        }
        if (std::any_of(l.constBegin(), l.constEnd(), [destCountry](const QVariant &v) {
                return v.value<LocationInformation>().isoCode() == destCountry;
            })) {
            continue;
        }

        LocationInformation info;
        info.setIsoCode(m_homeCountry);
        info.setIsoCode(destCountry);
        if (info.powerPlugCompatibility() != LocationInformation::FullyCompatible) {
            l.push_back(QVariant::fromValue(info));
        }
    }

    return l;
}

QStringList TripGroupController::currencies() const
{
    if (!m_tripGroupModel) {
        return {};
    }

    const auto group = m_tripGroupModel->tripGroupManager()->tripGroup(m_tgId);
    const auto elems = group.elements();
    QStringList l;

    for (const auto &resId : elems) {
        const auto res = m_tripGroupModel->tripGroupManager()->reservationManager()->reservation(resId);
        if (!LocationUtil::isLocationChange(res)) {
            continue;
        }

        const auto destCountry = LocationUtil::address(LocationUtil::arrivalLocation(res)).addressCountry();
        const auto currency = KCountry::fromAlpha2(destCountry).currencyCode();
        if (currency.isEmpty() || currency == m_homeCurrency || l.contains(currency)) {
            continue;
        }
        l.push_back(currency);
    }

    return l;
}

bool TripGroupController::canMerge() const
{
    if (!m_tripGroupModel) {
        return false;
    }

    return !m_tripGroupModel->adjacentTripGroups(m_tgId).isEmpty();
}

bool TripGroupController::canSplit() const
{
    if (!m_tripGroupModel) {
        return false;
    }

    return m_tripGroupModel->tripGroupManager()->tripGroup(m_tgId).elements().size() > 1;
}

#include "moc_tripgroupcontroller.cpp"
