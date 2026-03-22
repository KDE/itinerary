/*
    SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "scamwarningmanager.h"

#include <KItinerary/Place>

#include <QSettings>

using namespace Qt::Literals;
using namespace KItinerary;

/*

What should be included here:
Locations using a name in marketing that suggests proximity to a (large) city but that is actually
much further away and poorly connected to that city than other options to travel there.

Non-exclusive list of indicators for this being the case:
- 50km or even 100km distance to the city
- A "naming controversy" section on the corresponding Wikipedia page
- No or very limited public transport connectivity into the city
- Predominantly/only served by low(est)-cost carriers

*/

static constexpr const QLatin1StringView scam_airports[] = {
    "BVA"_L1, // pretends to be in Paris
    "CRL"_L1, // pretends to be in Brussels
    "HHN"_L1, // pretends to be in Frankfurt
    "NRN"_L1, // pretends to be in Düsseldorf
};

static constexpr const QLatin1StringView scam_ports[] = {
    "Tanger Med"_L1, // pretends to be Tanger
};

bool ScamWarningManager::warnForPlace(const QVariant &place, const QString &tripId)
{
    if (tripId.isEmpty()) {
        return false;
    }

    if (JsonLd::isA<Airport>(place)) {
        const auto iataCode = place.value<Airport>().iataCode();
        if (iataCode.size() != 3 || !std::ranges::binary_search(scam_airports, iataCode)) {
            return false;
        }
        QSettings settings;
        return settings.value("ScamWarnings/Trips/"_L1 + tripId + "/Airports/IATA/"_L1 + iataCode, true).toBool()
            && settings.value("ScamWarnings/Airports/IATA/"_L1 + iataCode, true).toBool();
    }

    if (JsonLd::isA<BoatTerminal>(place)) {
        const auto name = place.value<BoatTerminal>().name();
        if (name.isEmpty() || !std::binary_search(std::begin(scam_ports), std::end(scam_ports), name, [](auto &&lhs, auto &&rhs) {
            return QString::compare(lhs, rhs, Qt::CaseInsensitive) < 0;
        })) {
            return false;
        }
        QSettings settings;
        return settings.value("ScamWarnings/Trips/"_L1 + tripId + "/Port/Name/"_L1 + name, true).toBool()
            && settings.value("ScamWarnings/Port/Name/"_L1 + name, true).toBool();
    }

    return false;
}

void ScamWarningManager::ignorePlaceForTrip(const QVariant &place, const QString &tripId)
{
    QSettings settings;
    if (JsonLd::isA<Airport>(place)) {
        const auto iataCode = place.value<Airport>().iataCode();
        settings.setValue("ScamWarnings/Trips/"_L1 + tripId + "/Airports/IATA/"_L1 + iataCode, false);
    }
    else if (JsonLd::isA<BoatTerminal>(place)) {
        const auto name = place.value<BoatTerminal>().name();
        settings.setValue("ScamWarnings/Trips/"_L1 + tripId + "/Port/Name/"_L1 + name, false);
    }
}

void ScamWarningManager::ignorePlacePermanently(const QVariant &place)
{
    QSettings settings;
    if (JsonLd::isA<Airport>(place)) {
        const auto iataCode = place.value<Airport>().iataCode();
        settings.setValue("ScamWarnings/Airports/IATA/"_L1 + iataCode, false);
    }
    else if (JsonLd::isA<BoatTerminal>(place)) {
        const auto name = place.value<BoatTerminal>().name();
        settings.setValue("ScamWarnings/Port/Name/"_L1 + name, false);
    }
}

void ScamWarningManager::tripRemoved(const QString &tripId)
{
    QSettings settings;
    settings.remove("ScamWarnings/Trips/"_L1 + tripId);
}

#include "moc_scamwarningmanager.cpp"
