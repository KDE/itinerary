/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "locationhelper.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/Place>

#include <KCountry>
#include <KCountrySubdivision>

#include <QVariant>

using namespace KItinerary;

QString LocationHelper::departureCountry(const QVariant &res)
{
    if (LocationUtil::isLocationChange(res)) {
        return LocationUtil::address(LocationUtil::departureLocation(res)).addressCountry();
    }
    return LocationUtil::address(LocationUtil::location(res)).addressCountry();
}

QString LocationHelper::destinationCountry(const QVariant &res)
{
    if (LocationUtil::isLocationChange(res)) {
        return LocationUtil::address(LocationUtil::arrivalLocation(res)).addressCountry();
    }
    return LocationUtil::address(LocationUtil::location(res)).addressCountry();
}

QString LocationHelper::regionCode(const QVariant &loc)
{
    const auto addr = LocationUtil::address(loc);
    const auto coord = LocationUtil::geo(loc);

    QString code = addr.addressCountry();
    if (!code.isEmpty() && !addr.addressRegion().isEmpty()) {
        code += QLatin1Char('-') + addr.addressRegion();
        if (KCountrySubdivision::fromCode(code).isValid()) {
            return code;
        }
    }

    // prefer coordinate lookup over explicitly specified values if those don't result in a valid machine-readable code
    if (coord.isValid()) {
        const auto cs = KCountrySubdivision::fromLocation(coord.latitude(), coord.longitude());
        if (cs.isValid()) {
            return cs.code();
        }
    }

    return code;
}

double LocationHelper::distance(const QVariant &res)
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
