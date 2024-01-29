/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALENDARCORE_JNI_CALENDARPLUGIN_H
#define KCALENDARCORE_JNI_CALENDARPLUGIN_H

#include "calendardata.h"

#include "kandroidextras/jnitypes.h"
#include "kandroidextras/jnimethod.h"
#include "kandroidextras/jniproperty.h"
#include "kandroidextras/javatypes.h"
#include "kandroidextras/androidtypes.h"

JNI_TYPE(org, kde, kcalendarcore, CalendarPlugin)

/** JNI wrapper for the CalendarPlugin class. */
class JniCalendarPlugin {
    JNI_OBJECT(JniCalendarPlugin, org::kde::kcalendarcore::CalendarPlugin)
public:
    JNI_CONSTRUCTOR(JniCalendarPlugin, KAndroidExtras::android::content::Context)
    JNI_METHOD(KAndroidExtras::Jni::Array<JniCalendarData>, getCalendars)
};

#endif
