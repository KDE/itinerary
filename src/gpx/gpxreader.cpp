/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gpxreader.h"

bool Gpx::Reader::isRootElement() const
{
    return isStartElement() && name() == QLatin1StringView("gpx");
}

bool Gpx::Reader::isWaypointStart() const
{
    return isStartElement() && name() == QLatin1StringView("wpt");
}

bool Gpx::Reader::isWaypointEnd() const
{
    return isEndElement() && name() == QLatin1StringView("wpt");
}

float Gpx::Reader::latitude() const
{
    return attributes().value(QLatin1StringView("lat")).toFloat();
}

float Gpx::Reader::longitude() const
{
    return attributes().value(QLatin1StringView("lon")).toFloat();
}

bool Gpx::Reader::isGpxName() const
{
    return isStartElement() && name() == QLatin1StringView("name");
}

QString Gpx::Reader::gpxName()
{
    return readElementText();
}

bool Gpx::Reader::isGpxType() const
{
    return isStartElement() && name() == QLatin1StringView("type");
}

QString Gpx::Reader::gpxType()
{
    return readElementText();
}
