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

    if ((addr.addressRegion().isEmpty() || addr.addressCountry().isEmpty()) && coord.isValid()) {
        const auto cs = KCountrySubdivision::fromLocation(coord.latitude(), coord.longitude());
        if (cs.isValid()) {
            return cs.code();
        }
    }

    if (addr.addressCountry().isEmpty()) {
        return {};
    }

    if (addr.addressRegion().isEmpty()) {
        return addr.addressCountry();
    }
    return addr.addressCountry() + QLatin1Char('-') + addr.addressRegion();
}
