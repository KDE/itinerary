/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef TIMELINEELEMENT_H
#define TIMELINEELEMENT_H

#include <QDateTime>
#include <QString>
#include <QVariant>

class Transfer;

/** TimelineModel items. */
class TimelineElement {
    Q_GADGET

public:
    // Note: the order in here defines the priority of element if they occur at the same time
    enum ElementType {
        Undefined,
        TodayMarker,
        TripGroup,
        Transfer,
        WeatherForecast,
        LocationInfo,
        Flight,
        TrainTrip,
        CarRental,
        BusTrip,
        Restaurant,
        TouristAttraction,
        Event,
        Hotel
    };
    Q_ENUM(ElementType)

    // indicates whether an element is self-contained or the beginning/end of a longer timespan/range
    enum RangeType {
        SelfContained,
        RangeBegin,
        RangeEnd
    };
    Q_ENUM(RangeType)

    explicit TimelineElement();
    explicit TimelineElement(ElementType type, const QDateTime &dateTime, const QVariant &data = {});
    explicit TimelineElement(const QString &resId, const QVariant &res, RangeType rt);
    explicit TimelineElement(const ::Transfer &transfer);

    /** Timeline order. This considers only position in the timeline, not content. */
    bool operator<(const TimelineElement &other) const;
    bool operator==(const TimelineElement &other) const;

    /** Element is holds a reservation type. */
    bool isReservation() const;

    /** The time @p res is added to the timeline, for range type @p range. */
    static QDateTime relevantDateTime(const QVariant &res, TimelineElement::RangeType range);

    QString batchId; // reservation batch id
    QVariant content; // non-reservation content
    QDateTime dt; // relevant date/time
    ElementType elementType = Undefined;
    RangeType rangeType = SelfContained;
};

Q_DECLARE_METATYPE(TimelineElement)

#endif // TIMELINEELEMENT_H
