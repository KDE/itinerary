/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#include "tripgroupinfoprovider.h"

#include "countryinformation.h"
#include "reservationmanager.h"
#include "tripgroup.h"

#include <weatherforecast.h>
#include <weatherforecastmanager.h>

#include <KItinerary/LocationUtil>
#include <KItinerary/Place>
#include <KItinerary/SortUtil>

#include <QDateTime>
#include <QDebug>

using namespace KItinerary;

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

    const auto elems = group.elements();
    QVariant startRes;
    for (const auto &resId : elems) {
        const auto res = m_resMgr->reservation(resId);
        if (!LocationUtil::isLocationChange(res)) {
            continue;
        }
        if (startRes.isValid()) {
            const auto geo = LocationUtil::geo(LocationUtil::arrivalLocation(startRes));
            const auto startDt = SortUtil::endDateTime(startRes);
            const auto endDt = SortUtil::startDateTime(res);
            if (geo.isValid() && startDt.isValid() && endDt.isValid()) {
                const auto newFc = m_weatherMgr->forecast(geo.latitude(), geo.longitude(), startDt, endDt);
                if (!fc.isValid() && newFc.isValid()) {
                    fc.setDateTime(startDt);
                    fc.setRange(std::numeric_limits<int>::max()); // always consider the sub-range forecast during merging
                }
                fc.merge(newFc);
            }
        }
        startRes = res;
    }

    return fc;
}

QVariantList TripGroupInfoProvider::countryInformation(const TripGroup &group, const QString &homeCountryIsoCode) const
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
        if (std::any_of(l.constBegin(), l.constEnd(), [destCountry](const QVariant &v) { return v.value<CountryInformation>().isoCode() == destCountry; })) {
            continue;
        }

        CountryInformation info;
        info.setIsoCode(homeCountryIsoCode);
        info.setIsoCode(destCountry);
        if (info.powerPlugCompatibility() != CountryInformation::FullyCompatible) {
            l.push_back(QVariant::fromValue(info));
        }
    }

    return l;
}

#include "moc_tripgroupinfoprovider.cpp"
