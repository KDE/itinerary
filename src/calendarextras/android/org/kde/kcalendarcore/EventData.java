/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.kcalendarcore;

/** Struct for transferring event data over JNI. */
public class EventData
{
    public EventData() {}
    public int hashCode() { return (int)(id % 0xffff); }
    public boolean equals(java.lang.Object object)
    {
        EventData other = (EventData)object;
        return other != null && id == other.id;
    }

    public long id;
    public java.lang.String organizer;
    public java.lang.String title;
    public java.lang.String location;
    public java.lang.String description;
    // EVENT_COLOR?
    public long dtStart;
    public long dtEnd;
    public java.lang.String startTimezone;
    public java.lang.String endTimezone;
    public java.lang.String duration;
    public boolean allDay;
    public java.lang.String rrule;
    public java.lang.String rdate;
    public java.lang.String exrule;
    public java.lang.String exdate;
    public java.lang.String originalId;
    public long instanceId;
    // ORIGINAL_ALL_DAY - exception events?
    public int accessLevel;
    public int availability;
    // GUEST_CAN_X
    // CUSTOM_APP_PACKAGE|CUSTOM_APP_URI
    public java.lang.String uid2445;

    public AttendeeData[] attendees;
    public ExtendedPropertyData[] extendedProperties;
    public ReminderData[] reminders;
}
