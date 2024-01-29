/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tripgroupinfoprovider.h"

#include "locationinformation.h"
#include "reservationmanager.h"
#include "tripgroup.h"

#include "weatherforecast.h"
#include "weatherforecastmanager.h"

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

TripGroupInfoProvider::TripGroupInfoProvider() = default;
TripGroupInfoProvider::~TripGroupInfoProvider() = default;

void TripGroupInfoProvider::setReservationManager(ReservationManager *resMgr)
{
    m_resMgr = resMgr;
}

void TripGroupInfoProvider::setWeatherForecastManager(WeatherForecastManager *mgr)
{
    m_weatherMgr = mgr;
}

WeatherForecast TripGroupInfoProvider::weatherForecast(const TripGroup &group) const
{
    WeatherForecast fc;
    if (!m_weatherMgr->allowNetworkAccess()) {
        return fc;
    }

    const auto elems = group.elements();
    QVariant startRes;
    QDateTime lastEndTime = group.beginDateTime();

    for (const auto &resId : elems) {
        const auto res = m_resMgr->reservation(resId);
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

    return fc;
}

QVariantList TripGroupInfoProvider::locationInformation(const TripGroup &group, const QString &homeCountryIsoCode) const
{
    QVariantList l;

    const auto elems = group.elements();
    for (const auto &resId : elems) {
        const auto res = m_resMgr->reservation(resId);
        if (!LocationUtil::isLocationChange(res)) {
            continue;
        }

        const auto destCountry = LocationUtil::address(LocationUtil::arrivalLocation(res)).addressCountry();
        if (destCountry.isEmpty() || destCountry == homeCountryIsoCode) {
            continue;
        }
        if (std::any_of(l.constBegin(), l.constEnd(), [destCountry](const QVariant &v) { return v.value<LocationInformation>().isoCode() == destCountry; })) {
            continue;
        }

        LocationInformation info;
        info.setIsoCode(homeCountryIsoCode);
        info.setIsoCode(destCountry);
        if (info.powerPlugCompatibility() != LocationInformation::FullyCompatible) {
            l.push_back(QVariant::fromValue(info));
        }
    }

    return l;
}

QStringList TripGroupInfoProvider::currencies(const TripGroup& group, const QString &homeCurrency) const
{
    QStringList l;
    const auto elems = group.elements();
    for (const auto &resId : elems) {
        const auto res = m_resMgr->reservation(resId);
        if (!LocationUtil::isLocationChange(res)) {
            continue;
        }

        const auto destCountry = LocationUtil::address(LocationUtil::arrivalLocation(res)).addressCountry();
        const auto currency = KCountry::fromAlpha2(destCountry).currencyCode();
        if (currency.isEmpty() || currency == homeCurrency || l.contains(currency)) {
            continue;
        }
        l.push_back(currency);
    }

    return l;
}

#include "moc_tripgroupinfoprovider.cpp"
