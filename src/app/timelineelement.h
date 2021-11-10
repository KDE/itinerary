/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
        WeatherForecast,
        LocationInfo,
        Transfer,
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
    bool operator<(const QDateTime &otherDt) const;
    bool operator==(const TimelineElement &other) const;

    /** Element is holds a reservation type. */
    bool isReservation() const;

    /** Batch id of the contained reservation or transfer. */
    QString batchId() const;
    /** Content data.
     * @note For reservations this is also the batchId.
     */
    QVariant content() const;
    void setContent(const QVariant &content);

    /** The time @p res is added to the timeline, for range type @p range. */
    static QDateTime relevantDateTime(const QVariant &res, TimelineElement::RangeType range);

    QDateTime dt; // relevant date/time
    ElementType elementType = Undefined;
    RangeType rangeType = SelfContained;

private:
    QVariant m_content;
};

Q_DECLARE_METATYPE(TimelineElement)

#endif // TIMELINEELEMENT_H
