/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TIMELINEELEMENT_H
#define TIMELINEELEMENT_H

#include <QDateTime>
#include <QString>
#include <QVariant>

class TimelineModel;
class Transfer;

namespace KItinerary
{
class GeoCoordinates;
}

/** TimelineModel items. */
class TimelineElement
{
    Q_GADGET

public:
    // Note: the order in here defines the priority of element if they occur at the same time
    enum ElementType {
        Undefined,
        TodayMarker,
        WeatherForecast,
        LocationInfo,
        Transfer,
        Flight,
        TrainTrip,
        CarRental,
        BusTrip,
        BoatTrip,
        Restaurant,
        TouristAttraction,
        Event,
        Hotel
    };
    Q_ENUM(ElementType)

    // indicates whether an element is self-contained or the beginning/end of a longer timespan/range
    enum RangeType { SelfContained, RangeBegin, RangeEnd };
    Q_ENUM(RangeType)

    explicit TimelineElement();
    explicit TimelineElement(TimelineModel *model, ElementType type, const QDateTime &dateTime, const QVariant &data = {});
    explicit TimelineElement(TimelineModel *model, const QString &resId, const QVariant &res, RangeType rt);
    explicit TimelineElement(TimelineModel *model, const ::Transfer &transfer);

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

    /** Returns @c true if this element changes our location after taking effect. */
    bool isLocationChange() const;

    /** Returns @c true if this is element has an exclusively assigned defined
     *  time range. That's location change elements or e.g. events with a fixed end time,
     *  all things for which an end time is meaningful.
     *  The opposite to this, besides informational elements, are hotel or rental car bookings for example.
     */
    bool isTimeBoxed() const;

    /** Returns @c true if this element is cancelled or otherwise not happening,
     *  and thus its effect (such as a location change) wont actually happen.
     */
    bool isCanceled() const;

    /** Return @c true if this is element is an informational/virtual item, rather than something
     *  tied to a real-world action.
     */
    bool isInformational() const;

    /** Destination location, ie. the location we are in when/after
     *  this element took effect.
     *  @return a KItinerary place-like vocabulary type.
     */
    QVariant destination() const;

    /** End or arrival time.
     *  For location changes this is the arrival time.
     *  For timeboxed elements this is the end time.
     */
    QDateTime endDateTime() const;

    /** The time @p res is added to the timeline, for range type @p range. */
    static QDateTime relevantDateTime(const QVariant &res, TimelineElement::RangeType range);

    QDateTime dt; // relevant date/time
    ElementType elementType = Undefined;
    RangeType rangeType = SelfContained;

private:
    QVariant m_content;
    TimelineModel *m_model = nullptr;
};

Q_DECLARE_METATYPE(TimelineElement)

#endif // TIMELINEELEMENT_H
