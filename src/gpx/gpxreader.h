/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef GPX_READER_H
#define GPX_READER_H

#include <QXmlStreamReader>

namespace Gpx
{

/** GPX reader. */
class Reader : public QXmlStreamReader
{
public:
    using QXmlStreamReader::QXmlStreamReader;

    /** Returns @c true for the top-level element. */
    bool isRootElement() const;

    /** Returns @c true if we are positioned on a waypoint start element. */
    bool isWaypointStart() const;
    /** Returns @c true if we are positioned on a waypoint end element. */
    bool isWaypointEnd() const;

    /** Returns the latitude of the current waypoint, route point or track point. */
    float latitude() const;
    /** Returns the longitude of the current waypoint, route point or track point. */
    float longitude() const;

    /** Returns @c true if the current element is a name property. */
    bool isGpxName() const;
    /** Returns the value of a GPX name property. */
    QString gpxName();
    /** Returns @c true if the current element is a GPX type property. */
    bool isGpxType() const;
    /** Returns the value of a GPX type property. */
    QString gpxType();
};

}

#endif // GPX_READER_H
