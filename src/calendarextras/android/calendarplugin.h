/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALENDARCORE_JNI_CALENDARPLUGIN_H
#define KCALENDARCORE_JNI_CALENDARPLUGIN_H

#include "calendardata.h"

#include "../kjniextras/kjnimethod.h"


/** JNI wrapper for the CalendarPlugin class. */
class JniCalendarPlugin : public QtJniTypes::JObject<JniCalendarPlugin>
{
public:
    KJNI_CONSTRUCTOR(JniCalendarPlugin, QtJniTypes::Context)
    KJNI_METHOD(QJniArray<JniCalendarData>, getCalendars)
};

KJNI_DECLARE_CLASS(org, kde, kcalendarcore, CalendarPlugin, JniCalendarPlugin)

#endif
