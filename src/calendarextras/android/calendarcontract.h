/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALENDARCOREEXTRAS_CALENDARCONTRACT_H
#define KCALENDARCOREEXTRAS_CALENDARCONTRACT_H

#include "../kjniextras/kjnimethod.h"
#include "../kjniextras/kjniproperty.h"

namespace CalendarContract {

/** CalendarContracts.EventColumns wrapper. */
class CalendarColumns : public QtJniTypes::JObject<CalendarColumns>
{
    KJNI_OBJECT(CalendarColumns)

    KJNI_CONSTANT(jint, CAL_ACCESS_CONTRIBUTOR)
    KJNI_CONSTANT(jint, CAL_ACCESS_EDITOR)
    KJNI_CONSTANT(jint, CAL_ACCESS_FREEBUSY)
    KJNI_CONSTANT(jint, CAL_ACCESS_NONE)
    KJNI_CONSTANT(jint, CAL_ACCESS_OVERRIDE)
    KJNI_CONSTANT(jint, CAL_ACCESS_OWNER)
    KJNI_CONSTANT(jint, CAL_ACCESS_READ)
    KJNI_CONSTANT(jint, CAL_ACCESS_RESPOND)
    KJNI_CONSTANT(jint, CAL_ACCESS_ROOT)
};

/** CalendarContracts.EventColumns wrapper. */
class EventsColumns : public QtJniTypes::JObject<EventsColumns>
{
    KJNI_OBJECT(EventsColumns)

    KJNI_CONSTANT(jint, ACCESS_CONFIDENTIAL)
    KJNI_CONSTANT(jint, ACCESS_DEFAULT)
    KJNI_CONSTANT(jint, ACCESS_PRIVATE)
    KJNI_CONSTANT(jint, ACCESS_PUBLIC)

    KJNI_CONSTANT(jint, AVAILABILITY_BUSY)
    KJNI_CONSTANT(jint, AVAILABILITY_FREE)
    KJNI_CONSTANT(jint, AVAILABILITY_TENTATIVE)
};

/** CalendarContracts.AttendeesColumns wrapper. */
class AttendeesColumns : public QtJniTypes::JObject<AttendeesColumns>
{
    KJNI_OBJECT(AttendeesColumns)

    KJNI_CONSTANT(jint, ATTENDEE_STATUS_ACCEPTED)
    KJNI_CONSTANT(jint, ATTENDEE_STATUS_DECLINED)
    KJNI_CONSTANT(jint, ATTENDEE_STATUS_INVITED)
    KJNI_CONSTANT(jint, ATTENDEE_STATUS_NONE)
    KJNI_CONSTANT(jint, ATTENDEE_STATUS_TENTATIVE)

    KJNI_CONSTANT(jint, RELATIONSHIP_ATTENDEE)
    KJNI_CONSTANT(jint, RELATIONSHIP_NONE)
    KJNI_CONSTANT(jint, RELATIONSHIP_ORGANIZER)
    KJNI_CONSTANT(jint, RELATIONSHIP_PERFORMER)
    KJNI_CONSTANT(jint, RELATIONSHIP_SPEAKER)

    KJNI_CONSTANT(jint, TYPE_NONE)
    KJNI_CONSTANT(jint, TYPE_OPTIONAL)
    KJNI_CONSTANT(jint, TYPE_REQUIRED)
    KJNI_CONSTANT(jint, TYPE_RESOURCE)
};

/** CalendarContract.RemindersColumns wrapper. */
class RemindersColumns : public QtJniTypes::JObject<RemindersColumns>
{
    KJNI_OBJECT(RemindersColumns)

    KJNI_CONSTANT(jint, METHOD_ALARM)
    KJNI_CONSTANT(jint, METHOD_ALERT)
    KJNI_CONSTANT(jint, METHOD_DEFAULT)
    KJNI_CONSTANT(jint, METHOD_EMAIL)
    KJNI_CONSTANT(jint, METHOD_SMS)
};

}

KJNI_DECLARE_NESTED_CLASS(android, provider, CalendarContract, CalendarColumns, CalendarContract::CalendarColumns)
KJNI_DECLARE_NESTED_CLASS(android, provider, CalendarContract, EventsColumns, CalendarContract::EventsColumns)
KJNI_DECLARE_NESTED_CLASS(android, provider, CalendarContract, AttendeesColumns, CalendarContract::AttendeesColumns)
KJNI_DECLARE_NESTED_CLASS(android, provider, CalendarContract, RemindersColumns, CalendarContract::RemindersColumns)

#endif
