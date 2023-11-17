/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "weatherinformation.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/Place>

using namespace KItinerary;

QString WeatherInformation::labelForPlace(const QVariant &place)
{
    // TODO add fallbacks to region if we have no better name
    const auto addr = LocationUtil::address(place);
    if (!addr.addressLocality().isEmpty()) {
        return addr.addressLocality();
    }
    return LocationUtil::name(place);
}

#include "moc_weatherinformation.cpp"
