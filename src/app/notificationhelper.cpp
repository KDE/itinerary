/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "notificationhelper.h"
#include "logging.h"

#include <KPublicTransport/Stopover>

#include <KLocalizedString>

using namespace KPublicTransport;

static bool isSignificantDelayChange(int oldDelay, int newDely)
{
    // TODO we could do this relatively: 60 -> 62 matters less than 0 -> 2, for example
    return std::abs(oldDelay - newDely) > 2;
}

bool NotificationHelper::shouldNotify(const Stopover &oldStop, const Stopover &newStop, LiveData::Type context)
{
    // disruption state changed
    if (oldStop.disruptionEffect() != newStop.disruptionEffect()) {
        return true;
    }

    // platform changed
    if (newStop.hasExpectedPlatform() && oldStop.expectedPlatform() != newStop.expectedPlatform()) {
        return true;
    }

    // delay changed
    if (context == LiveData::Departure && newStop.hasExpectedDepartureTime() && isSignificantDelayChange(oldStop.departureDelay(), newStop.departureDelay())) {
        return true;
    }
    if (context == LiveData::Arrival && newStop.hasExpectedArrivalTime() && isSignificantDelayChange(oldStop.arrivalDelay(), newStop.arrivalDelay())) {
        return true;
    }

    return false;
}

static QString lineName(const LiveData &data)
{
    if (!data.departure.route().line().name().isEmpty()) {
        return data.departure.route().line().name();
    }
    if (!data.arrival.route().line().name().isEmpty()) {
        return data.arrival.route().line().name();
    }
    qCWarning(Log) << "Trying to create notification but no line name available!?";
    return {};
}

QString NotificationHelper::title(const LiveData &data)
{
    if (data.departure.disruptionEffect() != Disruption::NormalService || data.arrival.disruptionEffect() != Disruption::NormalService) {
        return i18n("Disruption on %1", lineName(data));
    }

    const auto platformChange = data.departure.platformChanged() || data.arrival.platformChanged();
    const auto oneDelay = data.departure.departureDelay() != 0 || data.arrival.arrivalDelay() > 0;
    const auto multiDelay = data.departure.departureDelay() != 0 && data.arrival.arrivalDelay() > 0;

    if (platformChange && oneDelay) {
        return i18n("Changes on %1", lineName(data));
    }

    if (multiDelay) {
        return i18n("Delays on %1", lineName(data));
    }
    if (data.departure.departureDelay() > 0) {
        return i18n("Delayed departure on %1", lineName(data));
    }
    if (data.departure.departureDelay() < 0) {
        return i18n("Earlier departure on %1", lineName(data));
    }
    if (data.arrival.arrivalDelay() > 0) {
        return i18n("Delayed arrival on %1", lineName(data));
    }

    if (platformChange) {
        return i18n("Platform change on %1", lineName(data));
    }

    return {};
}

QString NotificationHelper::message(const LiveData &data)
{
    QStringList msgs;
    if (data.departure.disruptionEffect() == Disruption::NoService) {
        msgs.push_back(i18n("Trip has been cancelled."));
    } else if (data.arrival.disruptionEffect() == Disruption::NoService) {
        msgs.push_back(i18n("Arrival has been cancelled."));
    }

    if (data.departure.departureDelay() > 0) {
        msgs.push_back(i18n("New departure time is: %1 (+%2)", QLocale().toString(data.departure.expectedDepartureTime().time()), data.departure.departureDelay()));
    } else if (data.departure.departureDelay() < 0) {
        msgs.push_back(i18n("New departure time is: %1 (%2)", QLocale().toString(data.departure.expectedDepartureTime().time()), data.departure.departureDelay()));
    }

    if (data.arrival.arrivalDelay() > 0) {
        msgs.push_back(i18n("New arrival time is: %1 (+%2)", QLocale().toString(data.arrival.expectedArrivalTime().time()), data.arrival.arrivalDelay()));
    }

    if (data.departure.platformChanged()) {
        msgs.push_back(i18n("New departure platform is: %1", data.departure.expectedPlatform()));
    }
    if (data.arrival.platformChanged()) {
        msgs.push_back(i18n("New arrival platform is: %1", data.arrival.expectedPlatform()));
    }

    return msgs.join(QLatin1Char('\n'));
}
