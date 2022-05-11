/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ANDROIDICALCONVERTER_H
#define ANDROIDICALCONVERTER_H

#include <KCalendarCore/Event>

class JniAttendeeData;
class JniEventData;
class JniExtendedPropertyData;
class JniReminderData;

/** Functions to convert between the Android and KCalendarCore iCal data representations. */
class AndroidIcalConverter
{
public:
    /** Convert Android event data to KCalendarCore::Event. */
    static KCalendarCore::Event::Ptr readEvent(const JniEventData &data);

    /** Convert an KCalendarCore::Event to a Android event data. */
    static JniEventData writeEvent(const KCalendarCore::Event::Ptr &event);

private:
    friend class AndroidIcalConverterTest;

    /** Convert Android attendee data to KCalndarCore::Attendee. */
    static KCalendarCore::Attendee readAttendee(const JniAttendeeData &data);
    /** Convert KCalendarCore::Attendee to Android attendee data. */
    static JniAttendeeData writeAttendee(const KCalendarCore::Attendee &attendee);

    /** Convert Android reminder data to KCalendarCore::Alarm. */
    static KCalendarCore::Alarm::Ptr readAlarm(const JniReminderData &data, KCalendarCore::Incidence *parent);
    /** Convert KCalendarCore::Alarm to Android reminder data. */
    static JniReminderData writeAlarm(const KCalendarCore::Alarm::Ptr &alarm);

    /** Add a decoded DAVx⁵ extended property to @p incidence. */
    static void addExtendedProperty(KCalendarCore::Incidence *incidence, const QString &name, const QString &value);
    /** Write extended properties in DAVx⁵ format. */
    static std::vector<JniExtendedPropertyData> writeExtendedProperties(const KCalendarCore::Incidence *incidence);

    /** Parse RDATE/EXDATE Android encoding. */
    template <typename T>
    static QList<T> readRDates(const QString &data);
    /** Write RDATE/EXDATE entries in the Android format. */
    static QString writeRDates(const QList<QDate> &rdates);
    static QString writeRDates(const QList<QDateTime> &rdates);
};

#endif // ANDROIDICALCONVERTER_H
