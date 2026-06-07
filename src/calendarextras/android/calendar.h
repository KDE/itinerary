/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALENDARCORE_JNI_CALENDAR_H
#define KCALENDARCORE_JNI_CALENDAR_H

#include "calendardata.h"
#include "eventdata.h"

#include "../kjniextras/kjnimethod.h"
#include "../kjniextras/kjniproperty.h"


/** JNI wrapper for the CalendarPlugin class. */
class JniCalendar : public QtJniTypes::JObject<JniCalendar>
{
    KJNI_OBJECT(JniCalendar)
public:
    KJNI_CONSTRUCTOR(JniCalendar, QtJniTypes::Context, jlong)
    KJNI_METHOD(QJniArray<JniEventData>, rawEvents)
    KJNI_METHOD(QJniArray<JniEventData>, rawEvents, jlong, jlong, bool)
    KJNI_METHOD(bool, addEvent, JniEventData)
    KJNI_METHOD(JniEventData, event, QString)
    KJNI_METHOD(JniEventData, event, QString, jlong)
    KJNI_METHOD(bool, deleteEvent, QString)
    KJNI_METHOD(bool, deleteEvent, QString, jlong)
    KJNI_METHOD(bool, deleteEventInstances, QString)
    KJNI_METHOD(QJniArray<JniEventData>, eventInstances, QString)
    KJNI_METHOD(bool, updateEvent, JniEventData, bool, bool)
};

KJNI_DECLARE_CLASS(org, kde, kcalendarcore, Calendar, JniCalendar)

#endif
