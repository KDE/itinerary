/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WEATHERINFORMATION_H
#define WEATHERINFORMATION_H

#include <weatherforecast.h>

#include <QString>

/** Holds weather information for display in the timeline. */
class WeatherInformation
{
    Q_GADGET
    Q_PROPERTY(WeatherForecast forecast MEMBER forecast CONSTANT)
    Q_PROPERTY(QString locationName MEMBER locationName CONSTANT)
public:
    WeatherForecast forecast;
    QString locationName;

    static QString labelForPlace(const QVariant &place);
};

Q_DECLARE_METATYPE(WeatherInformation)

#endif // WEATHERINFORMATION_H
