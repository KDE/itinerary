/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALENDARCORE_JNI_CALENDARDATA_H
#define KCALENDARCORE_JNI_CALENDARDATA_H

#include <kandroidextras/jniobject.h>
#include <kandroidextras/jniproperty.h>
#include <kandroidextras/jnitypes.h>
#include <kandroidextras/javatypes.h>

JNI_TYPE(org, kde, kcalendarcore, CalendarData)

/** JNI wrapper for the CalendarPlugin class. */
class JniCalendarData {
    JNI_OBJECT(JniCalendarData, org::kde::kcalendarcore::CalendarData)
public:
    JNI_PROPERTY(jlong, id)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, displayName)
    JNI_PROPERTY(jint, accessLevel)
    JNI_PROPERTY(jint, color)
    JNI_PROPERTY(KAndroidExtras::java::lang::String, timezone)
};

#endif
