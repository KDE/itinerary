/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALENDARCORE_JNI_CALENDAR_H
#define KCALENDARCORE_JNI_CALENDAR_H

#include "eventdata.h"

#include "kandroidextras/androidtypes.h"
#include "kandroidextras/javatypes.h"
#include "kandroidextras/jnimethod.h"
#include "kandroidextras/jniobject.h"
#include "kandroidextras/jniproperty.h"
#include "kandroidextras/jnitypes.h"

JNI_TYPE(org, kde, kcalendarcore, Calendar)

/** JNI wrapper for the CalendarPlugin class. */
class JniCalendar
{
    JNI_OBJECT(JniCalendar, org::kde::kcalendarcore::Calendar)
public:
    JNI_CONSTRUCTOR(JniCalendar, KAndroidExtras::android::content::Context, jlong)
    JNI_METHOD(Jni::Array<JniEventData>, rawEvents)
    JNI_METHOD(Jni::Array<JniEventData>, rawEvents, jlong, jlong, bool)
    JNI_METHOD(bool, addEvent, JniEventData)
    JNI_METHOD(JniEventData, event, KAndroidExtras::java::lang::String)
    JNI_METHOD(JniEventData, event, KAndroidExtras::java::lang::String, jlong)
    JNI_METHOD(bool, deleteEvent, KAndroidExtras::java::lang::String)
    JNI_METHOD(bool, deleteEvent, KAndroidExtras::java::lang::String, jlong)
    JNI_METHOD(bool, deleteEventInstances, KAndroidExtras::java::lang::String)
    JNI_METHOD(Jni::Array<JniEventData>, eventInstances, KAndroidExtras::java::lang::String)
    JNI_METHOD(bool, updateEvent, JniEventData, bool, bool)
};

#endif
