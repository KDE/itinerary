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
    // TODO add fallbacks to region or place name
    const auto addr = LocationUtil::address(place);
    return addr.addressLocality();
}
