/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALENDARCORE_JNI_CALENDARDATA_H
#define KCALENDARCORE_JNI_CALENDARDATA_H

#include "../kjniextras/kjnimethod.h"
#include "../kjniextras/kjniproperty.h"

/** JNI wrapper for the CalendarPlugin class. */
class JniCalendarData : public QtJniTypes::JObject<JniCalendarData>
{
    KJNI_OBJECT(JniCalendarData)
public:
    KJNI_PROPERTY(jlong, id)
    KJNI_PROPERTY(QString, displayName)
    KJNI_PROPERTY(jint, accessLevel)
    KJNI_PROPERTY(jint, color)
    KJNI_PROPERTY(QString, timezone)
    KJNI_PROPERTY(QString, owner)
};

KJNI_DECLARE_CLASS(org, kde, kcalendarcore, CalendarData, JniCalendarData)

#endif
