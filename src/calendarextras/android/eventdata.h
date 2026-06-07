/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALENDARCORE_JNI_EVENTDATA_H
#define KCALENDARCORE_JNI_EVENTDATA_H

#include "../kjniextras/kjnimethod.h"
#include "../kjniextras/kjniobject.h"
#include "../kjniextras/kjniproperty.h"

/** JNI wrapper for the event reminder data class. */
class JniReminderData : public QtJniTypes::JObject<JniReminderData>
{
    KJNI_OBJECT(JniReminderData)
public:
    KJNI_CONSTRUCTOR(JniReminderData)
    KJNI_PROPERTY(int, minutes)
    KJNI_PROPERTY(int, method)
};

KJNI_DECLARE_CLASS(org, kde, kcalendarcore, ReminderData, JniReminderData)

/** JNI wrapper for the event extended property data class. */
class JniExtendedPropertyData : public QtJniTypes::JObject<JniExtendedPropertyData>
{
    KJNI_OBJECT(JniExtendedPropertyData)
public:
    KJNI_CONSTRUCTOR(JniExtendedPropertyData)
    KJNI_PROPERTY(QString, name)
    KJNI_PROPERTY(QString, value)
};

KJNI_DECLARE_CLASS(org, kde, kcalendarcore, ExtendedPropertyData, JniExtendedPropertyData)

/** JNI wrapper for the event reminder data class. */
class JniAttendeeData : public QtJniTypes::JObject<JniAttendeeData>
{
    KJNI_OBJECT(JniAttendeeData)
public:
    KJNI_CONSTRUCTOR(JniAttendeeData)
    KJNI_PROPERTY(QString, name)
    KJNI_PROPERTY(QString, email)
    KJNI_PROPERTY(int, relationship)
    KJNI_PROPERTY(int, type)
    KJNI_PROPERTY(int, status)
};

KJNI_DECLARE_CLASS(org, kde, kcalendarcore, AttendeeData, JniAttendeeData)

/** JNI wrapper for the event data class. */
class JniEventData : public QtJniTypes::JObject<JniEventData>
{
    KJNI_OBJECT(JniEventData)
public:
    KJNI_CONSTRUCTOR(JniEventData)
    KJNI_PROPERTY(jlong, id)
    KJNI_PROPERTY(QString, organizer)
    KJNI_PROPERTY(QString, title)
    KJNI_PROPERTY(QString, location)
    KJNI_PROPERTY(QString, description)
    KJNI_PROPERTY(jlong, dtStart)
    KJNI_PROPERTY(jlong, dtEnd)
    KJNI_PROPERTY(QString, startTimezone)
    KJNI_PROPERTY(QString, endTimezone)
    KJNI_PROPERTY(QString, duration)
    KJNI_PROPERTY(bool, allDay)
    KJNI_PROPERTY(QString, rrule)
    KJNI_PROPERTY(QString, rdate)
    KJNI_PROPERTY(QString, exrule)
    KJNI_PROPERTY(QString, exdate)
    KJNI_PROPERTY(QString, originalId)
    KJNI_PROPERTY(jlong, instanceId)
    KJNI_PROPERTY(jint, accessLevel)
    KJNI_PROPERTY(jint, availability)
    KJNI_PROPERTY(QString, uid2445)

    KJNI_PROPERTY(QJniArray<JniAttendeeData>, attendees)
    KJNI_PROPERTY(QJniArray<JniExtendedPropertyData>, extendedProperties)
    KJNI_PROPERTY(QJniArray<JniReminderData>, reminders)
};

KJNI_DECLARE_CLASS(org, kde, kcalendarcore, EventData, JniEventData)

#endif
