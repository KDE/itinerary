/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALENDARCORE_JNI_EVENTDATA_H
#define KCALENDARCORE_JNI_EVENTDATA_H

#include "kandroidextras/javatypes.h"
#include "kandroidextras/jnimethod.h"
#include "kandroidextras/jniobject.h"
#include "kandroidextras/jniproperty.h"
#include "kandroidextras/jnitypes.h"

JNI_TYPE(org, kde, kcalendarcore, EventData)
JNI_TYPE(org, kde, kcalendarcore, AttendeeData)
JNI_TYPE(org, kde, kcalendarcore, ExtendedPropertyData)
JNI_TYPE(org, kde, kcalendarcore, ReminderData)

/** JNI wrapper for the event reminder data class. */
class JniReminderData
{
    JNI_OBJECT(JniReminderData, org::kde::kcalendarcore::ReminderData)
public:
    JNI_CONSTRUCTOR(JniReminderData)
    JNI_PROPERTY(int, minutes)
    JNI_PROPERTY(int, method)
};

/** JNI wrapper for the event extended property data class. */
class JniExtendedPropertyData
{
    JNI_OBJECT(JniExtendedPropertyData, org::kde::kcalendarcore::ExtendedPropertyData)
public:
    JNI_CONSTRUCTOR(JniExtendedPropertyData)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, name)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, value)
};

/** JNI wrapper for the event reminder data class. */
class JniAttendeeData
{
    JNI_OBJECT(JniAttendeeData, org::kde::kcalendarcore::AttendeeData)
public:
    JNI_CONSTRUCTOR(JniAttendeeData)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, name)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, email)
    JNI_PROPERTY(int, relationship)
    JNI_PROPERTY(int, type)
    JNI_PROPERTY(int, status)
};

/** JNI wrapper for the event data class. */
class JniEventData
{
    JNI_OBJECT(JniEventData, org::kde::kcalendarcore::EventData)
public:
    JNI_CONSTRUCTOR(JniEventData)
    JNI_PROPERTY(jlong, id)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, organizer)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, title)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, location)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, description)
    JNI_PROPERTY(jlong, dtStart)
    JNI_PROPERTY(jlong, dtEnd)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, startTimezone)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, endTimezone)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, duration)
    JNI_PROPERTY(bool, allDay)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, rrule)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, rdate)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, exrule)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, exdate)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, originalId)
    JNI_PROPERTY(jlong, instanceId)
    JNI_PROPERTY(jint, accessLevel)
    JNI_PROPERTY(jint, availability)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, uid2445)

    JNI_PROPERTY(KAndroidExtras::Jni::Array<JniAttendeeData>, attendees)
    JNI_PROPERTY(KAndroidExtras::Jni::Array<JniExtendedPropertyData>, extendedProperties)
    JNI_PROPERTY(KAndroidExtras::Jni::Array<JniReminderData>, reminders)
};

#endif
