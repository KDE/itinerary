/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "gpxreader.h"

bool Gpx::Reader::isRootElement() const
{
    return isStartElement() && name() == QLatin1String("gpx");
}

bool Gpx::Reader::isWaypointStart() const
{
    return isStartElement() && name() == QLatin1String("wpt");
}

bool Gpx::Reader::isWaypointEnd() const
{
    return isEndElement() && name() == QLatin1String("wpt");
}

float Gpx::Reader::latitude() const
{
    return attributes().value(QLatin1String("lat")).toFloat();
}

float Gpx::Reader::longitude() const
{
    return attributes().value(QLatin1String("lon")).toFloat();
}

bool Gpx::Reader::isGpxName() const
{
    return isStartElement() && name() == QLatin1String("name");
}

QString Gpx::Reader::gpxName()
{
    return readElementText();
}

bool Gpx::Reader::isGpxType() const
{
    return isStartElement() && name() == QLatin1String("type");
}

QString Gpx::Reader::gpxType()
{
    return readElementText();
}
