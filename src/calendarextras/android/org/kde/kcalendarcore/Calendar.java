/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.kcalendarcore;

import android.content.*;
import android.database.*;
import android.net.Uri;
import android.provider.*;
import android.util.Log;

import java.util.HashSet;

public class Calendar
{
    public Calendar(android.content.Context context, long id)
    {
        m_context = context;
        m_id = id;
    }

    private static final String TAG = "org.kde.kcalendarcore";

    // keep same field order as in INSTANCE_PROJECTION!
    private static final String[] EVENT_PROJECTION = new String[] {
        CalendarContract.Events._ID,
        CalendarContract.Events.ORGANIZER,
        CalendarContract.Events.TITLE,
        CalendarContract.Events.EVENT_LOCATION,
        CalendarContract.Events.DESCRIPTION,
        //CalendarContract.Events.EVENT_COLOR
        CalendarContract.Events.DTSTART,
        CalendarContract.Events.DTEND,
        CalendarContract.Events.EVENT_TIMEZONE,
        CalendarContract.Events.EVENT_END_TIMEZONE,
        CalendarContract.Events.DURATION,
        CalendarContract.Events.ALL_DAY,
        CalendarContract.Events.RRULE,
        CalendarContract.Events.RDATE,
        CalendarContract.Events.EXRULE,
        CalendarContract.Events.EXDATE,
        CalendarContract.Events.ORIGINAL_ID,
        CalendarContract.Events.ORIGINAL_INSTANCE_TIME,
        //CalendarContract.Events.ORIGINAL_ALL_DAY
        CalendarContract.Events.ACCESS_LEVEL,
        CalendarContract.Events.AVAILABILITY,
        //CalendarContract.Events.CUSTOM_APP_PACKAGE,
        //CalendarContract.Events.CUSTOM_APP_URI,
        CalendarContract.Events.UID_2445
    };

    private static final String[] ATTENDEE_PROJECTION = new String[] {
        CalendarContract.Attendees.ATTENDEE_NAME,
        CalendarContract.Attendees.ATTENDEE_EMAIL,
        CalendarContract.Attendees.ATTENDEE_RELATIONSHIP,
        CalendarContract.Attendees.ATTENDEE_TYPE,
        CalendarContract.Attendees.ATTENDEE_STATUS
    };

    private static final String[] EXTENDED_PROPERTY_PROJECTION = new String[] {
        CalendarContract.ExtendedProperties.NAME,
        CalendarContract.ExtendedProperties.VALUE
    };

    private static final String[] REMINDER_PROJECTION = new String[] {
        CalendarContract.Reminders.MINUTES,
        CalendarContract.Reminders.METHOD
    };

    // keep same field order as in EVENT_PROJECTION!
    private static final String[] INSTANCE_PROJECTION = new String[] {
        CalendarContract.Instances.EVENT_ID,
        CalendarContract.Instances.ORGANIZER,
        CalendarContract.Instances.TITLE,
        CalendarContract.Instances.EVENT_LOCATION,
        CalendarContract.Instances.DESCRIPTION,
        //CalendarContract.Instances.EVENT_COLOR
        CalendarContract.Instances.DTSTART,
        CalendarContract.Instances.DTEND,
        CalendarContract.Instances.EVENT_TIMEZONE,
        CalendarContract.Instances.EVENT_END_TIMEZONE,
        CalendarContract.Instances.DURATION,
        CalendarContract.Instances.ALL_DAY,
        CalendarContract.Instances.RRULE,
        CalendarContract.Instances.RDATE,
        CalendarContract.Instances.EXRULE,
        CalendarContract.Instances.EXDATE,
        CalendarContract.Instances.ORIGINAL_ID,
        CalendarContract.Instances.ORIGINAL_INSTANCE_TIME,
        //CalendarContract.Instances.ORIGINAL_ALL_DAY
        CalendarContract.Instances.ACCESS_LEVEL,
        CalendarContract.Instances.AVAILABILITY,
        //CalendarContract.Instances.CUSTOM_APP_PACKAGE,
        //CalendarContract.Instances.CUSTOM_APP_URI,
        CalendarContract.Instances.UID_2445
    };

    public EventData[] rawEvents()
    {
        ContentResolver cr = m_context.getContentResolver();

        String eventSelection = "(( " + CalendarContract.Events.CALENDAR_ID + " == " + m_id + " ))";
        Cursor cur = cr.query(CalendarContract.Events.CONTENT_URI, EVENT_PROJECTION, eventSelection, null, null);

        EventData result[] = new EventData[cur.getCount()];
        while (cur.moveToNext()) {
            result[cur.getPosition()] = eventFromCurser(cur);
        }

        for (int i = 0; i < result.length; ++i) {
            loadReminders(result[i]);
            loadAttendees(result[i]);
            loadExtendedProperties(result[i]);
        }

        return result;
    }

    public EventData[] rawEvents(long begin, long end, boolean inclusive)
    {
        ContentResolver cr = m_context.getContentResolver();

        Uri.Builder builder = CalendarContract.Instances.CONTENT_URI.buildUpon();
        ContentUris.appendId(builder, begin);
        ContentUris.appendId(builder, end);
        // TODO ^ is this inclusive or exclusive?

        String instanceSelection = "(( " + CalendarContract.Instances.CALENDAR_ID  + " == " + m_id + " ))";
        Cursor cur = cr.query(builder.build(), INSTANCE_PROJECTION, instanceSelection, null, null);

        Log.i(TAG, "Events in range: " + cur.getCount());
        HashSet<EventData> events = new HashSet<EventData>();
        while (cur.moveToNext()) {
            events.add(eventFromCurser(cur));
        }
        Log.i(TAG, "Unique events in range: " + events.size());

        EventData result[] = new EventData[events.size()];
        int i = 0;
        for (EventData data : events) {
            loadReminders(data);
            loadAttendees(data);
            loadExtendedProperties(data);
            result[i++] = data;
        }

        return result;
    }

    public boolean addEvent(EventData event)
    {
        Log.i(TAG, event.title + " " + event.allDay + " " + event.dtStart);
        ContentResolver cr = m_context.getContentResolver();
        ContentValues values = fillContentValues(event);
        // identification/add-only properties update isn't allowed to change
        if (event.originalId != null && !event.originalId.isEmpty()) {
            // TODO what is ORIGINAL_ID here?
            values.put(CalendarContract.Events.ORIGINAL_INSTANCE_TIME, event.instanceId);
        }
        values.put(CalendarContract.Events.CALENDAR_ID, m_id);
        values.put(CalendarContract.Events.UID_2445, event.uid2445);

        Uri uri = cr.insert(CalendarContract.Events.CONTENT_URI, values);
        long id = Long.parseLong(uri.getLastPathSegment());
        Log.i(TAG, "Event added: " + id);

        // insert reminders, attendees and extended properties
        if (event.reminders != null) {
            for (ReminderData data : event.reminders) {
                addReminder(data, id);
            }
        }
        if (event.attendees != null) {
            for (AttendeeData data : event.attendees) {
                addAttendee(data, id);
            }
        }
        /* ### not allowed for non-syncproviders :(
        if (event.extendedProperties != null) {
            for (ExtendedPropertyData data : event.extendedProperties) {
                addExtendedProperty(data, id);
            }
        }*/

        return true;
    }

    public EventData event(java.lang.String uid)
    {
        ContentResolver cr = m_context.getContentResolver();
        String eventSelection = "(( " + CalendarContract.Events.CALENDAR_ID + " == " + m_id + " ) AND ( "
            + CalendarContract.Events.UID_2445 + " == ? ) AND ("
            + CalendarContract.Events.ORIGINAL_ID + " is null))";
        String[] selectionArgs = new String[] { uid };
        Cursor cur = cr.query(CalendarContract.Events.CONTENT_URI, EVENT_PROJECTION, eventSelection, selectionArgs, null);
        if (cur.moveToNext()) {
            EventData result = eventFromCurser(cur);
            loadReminders(result);
            loadAttendees(result);
            loadExtendedProperties(result);
            return result;
        }
        return null;
    }

    public EventData event(java.lang.String uid, long instanceId)
    {
        ContentResolver cr = m_context.getContentResolver();
        String eventSelection = "(( " + CalendarContract.Events.CALENDAR_ID + " == " + m_id + " ) AND ( "
            + CalendarContract.Events.UID_2445 + " == ? ) AND ("
            + CalendarContract.Events.ORIGINAL_INSTANCE_TIME + " == " + instanceId + " ))";
        String[] selectionArgs = new String[] { uid };
        Cursor cur = cr.query(CalendarContract.Events.CONTENT_URI, EVENT_PROJECTION, eventSelection, selectionArgs, null);
        if (cur.moveToNext()) {
            EventData result = eventFromCurser(cur);
            loadReminders(result);
            loadAttendees(result);
            loadExtendedProperties(result);
            return result;
        }
        return null;
    }

    public boolean deleteEvent(java.lang.String uid)
    {
        ContentResolver cr = m_context.getContentResolver();
        String eventSelection = "(( " + CalendarContract.Events.CALENDAR_ID + " == " + m_id + " ) AND ( "
            + CalendarContract.Events.UID_2445 + " == ? ) AND ("
            + CalendarContract.Events.ORIGINAL_ID + " is null ))";
        String[] selectionArgs = new String[] { uid };
        return cr.delete(CalendarContract.Events.CONTENT_URI, eventSelection, selectionArgs) > 0;
    }

    public boolean deleteEvent(java.lang.String uid, long instanceId)
    {
        ContentResolver cr = m_context.getContentResolver();
        String eventSelection = "(( " + CalendarContract.Events.CALENDAR_ID + " == " + m_id + " ) AND ( "
            + CalendarContract.Events.UID_2445 + " == ? ) AND ("
            + CalendarContract.Events.ORIGINAL_INSTANCE_TIME + " == " + instanceId + " ))";
        String[] selectionArgs = new String[] { uid };
        return cr.delete(CalendarContract.Events.CONTENT_URI, eventSelection, selectionArgs) > 0;
    }

    public boolean deleteEventInstances(java.lang.String uid)
    {
        ContentResolver cr = m_context.getContentResolver();
        String eventSelection = "(( " + CalendarContract.Events.CALENDAR_ID + " == " + m_id + " ) AND ( "
            + CalendarContract.Events.UID_2445 + " == ? ))";
        String[] selectionArgs = new String[] { uid };
        return cr.delete(CalendarContract.Events.CONTENT_URI, eventSelection, selectionArgs) > 0;
    }

    public EventData[] eventInstances(java.lang.String uid)
    {
        ContentResolver cr = m_context.getContentResolver();

        String eventSelection = "(( " + CalendarContract.Events.CALENDAR_ID + " == " + m_id + " ) AND ( "
            + CalendarContract.Events.UID_2445 + " == ? ))";
        String[] selectionArgs = new String[] { uid };

        Cursor cur = cr.query(CalendarContract.Events.CONTENT_URI, EVENT_PROJECTION, eventSelection, selectionArgs, null);
        EventData result[] = new EventData[cur.getCount()];
        while (cur.moveToNext()) {
            result[cur.getPosition()] = eventFromCurser(cur);
        }

        for (int i = 0; i < result.length; ++i) {
            loadReminders(result[i]);
            loadAttendees(result[i]);
            loadExtendedProperties(result[i]);
        }

        return result;
    }

    public boolean updateEvent(EventData event, boolean remindersChanged, boolean attendeesChanged)
    {
        ContentResolver cr = m_context.getContentResolver();

        // determine event id
        String eventSelection = "(( " + CalendarContract.Events.CALENDAR_ID + " == " + m_id + " ) AND ( "
            + CalendarContract.Events.UID_2445 + " == ? ) AND (";
        if (event.originalId == null || event.originalId.isEmpty()) {
            eventSelection += CalendarContract.Events.ORIGINAL_ID + " is null ))";
        } else {
            eventSelection += CalendarContract.Events.ORIGINAL_INSTANCE_TIME + " == " + event.instanceId + " ))";
        }
        String[] selectionArgs = new String[] { event.uid2445 };
        Cursor cur = cr.query(CalendarContract.Events.CONTENT_URI, EVENT_PROJECTION, eventSelection, selectionArgs, null);
        if (cur.getCount() != 1 || !cur.moveToNext()) {
            Log.w(TAG, "unable to identify event to update: " + cur.getCount());
            return false;
        }
        long eventId = cur.getLong(0);
        Log.d(TAG, "found event to update: " + eventId);

        // apply event changes
        ContentValues values = fillContentValues(event);
        int rows = cr.update(ContentUris.withAppendedId(CalendarContract.Events.CONTENT_URI, eventId), values, null, null);
        if (rows != 1) {
            Log.w(TAG, "unable to update event: " + rows);
            return false;
        }

        // apply reminder changes
        if (remindersChanged) {
            deleteReminders(eventId);
            if (event.reminders != null) {
                for (ReminderData data : event.reminders) {
                    addReminder(data, eventId);
                }
            }
        }

        // apply attendee changes
        if (attendeesChanged) {
            deleteAttendees(eventId);
            if (event.attendees != null) {
                for (AttendeeData data : event.attendees) {
                    addAttendee(data, eventId);
                }
            }
        }

        return true;
    }

    private EventData eventFromCurser(Cursor c)
    {
        EventData data = new EventData();
        data.id = c.getLong(0);
        data.organizer = c.getString(1);
        data.title = c.getString(2);
        data.location = c.getString(3);
        data.description = c.getString(4);
        data.dtStart = c.getLong(5);
        data.dtEnd = c.getLong(6);
        data.startTimezone = c.getString(7);
        data.endTimezone = c.getString(8);
        data.duration = c.getString(9);
        data.allDay = c.getInt(10) == 1;
        data.rrule = c.getString(11);
        data.rdate = c.getString(12);
        data.exrule = c.getString(13);
        data.exdate = c.getString(14);
        data.originalId = c.getString(15);
        data.instanceId = c.getLong(16);
        data.accessLevel = c.getInt(17);
        data.availability = c.getInt(18);
        data.uid2445 = c.getString(19);
        return data;
    }

    private void loadReminders(EventData event)
    {
        ContentResolver cr = m_context.getContentResolver();

        String reminderSelection = "(( " + CalendarContract.Reminders.EVENT_ID + " == " + event.id + " ))";
        Cursor cur = cr.query(CalendarContract.Reminders.CONTENT_URI, REMINDER_PROJECTION, reminderSelection, null, null);

        int reminderCount = cur.getCount();
        if (reminderCount <= 0) {
            return;
        }

        event.reminders = new ReminderData[reminderCount];
        while (cur.moveToNext()) {
            ReminderData data = new ReminderData();
            data.minutes = cur.getInt(0);
            data.method = cur.getInt(1);
            event.reminders[cur.getPosition()] = data;
        }
    }

    private void loadAttendees(EventData event)
    {
        ContentResolver cr = m_context.getContentResolver();

        String attendeeSelection = "(( " + CalendarContract.Attendees.EVENT_ID + " == " + event.id + " ))";
        Cursor cur = cr.query(CalendarContract.Attendees.CONTENT_URI, ATTENDEE_PROJECTION, attendeeSelection, null, null);

        int attendeeCount = cur.getCount();
        if (attendeeCount <= 0) {
            return;
        }

        event.attendees = new AttendeeData[attendeeCount];
        while (cur.moveToNext()) {
            AttendeeData data = new AttendeeData();
            data.name = cur.getString(0);
            data.email = cur.getString(1);
            data.relationship = cur.getInt(2);
            data.type = cur.getInt(3);
            data.status = cur.getInt(4);
            event.attendees[cur.getPosition()] = data;
        }
    }

    private void loadExtendedProperties(EventData event)
    {
        ContentResolver cr = m_context.getContentResolver();

        String propSelection = "(( " + CalendarContract.ExtendedProperties.EVENT_ID + " == " + event.id + " ))";
        Cursor cur = cr.query(CalendarContract.ExtendedProperties.CONTENT_URI, EXTENDED_PROPERTY_PROJECTION, propSelection, null, null);

        int propCount = cur.getCount();
        if (propCount <= 0) {
            return;
        }

        event.extendedProperties = new ExtendedPropertyData[propCount];
        while (cur.moveToNext()) {
            ExtendedPropertyData data = new ExtendedPropertyData();
            data.name = cur.getString(0);
            data.value = cur.getString(1);
            event.extendedProperties[cur.getPosition()] = data;
        }
    }

    private ContentValues fillContentValues(EventData event)
    {
        ContentValues values = new ContentValues();
        values.put(CalendarContract.Events.ORGANIZER, event.organizer);
        values.put(CalendarContract.Events.TITLE, event.title);
        values.put(CalendarContract.Events.EVENT_LOCATION, event.location);
        values.put(CalendarContract.Events.DESCRIPTION, event.description);
        values.put(CalendarContract.Events.DTSTART, event.dtStart);
        values.put(CalendarContract.Events.DTEND, event.dtEnd);
        values.put(CalendarContract.Events.EVENT_TIMEZONE, event.startTimezone);
        values.put(CalendarContract.Events.EVENT_END_TIMEZONE, event.endTimezone);
        values.put(CalendarContract.Events.DURATION, event.duration);
        values.put(CalendarContract.Events.ALL_DAY, event.allDay ? 1 : 0);
        values.put(CalendarContract.Events.RRULE, event.rrule);
        values.put(CalendarContract.Events.RDATE, event.rdate);
        values.put(CalendarContract.Events.EXRULE, event.exrule);
        values.put(CalendarContract.Events.EXDATE, event.exdate);
        values.put(CalendarContract.Events.ACCESS_LEVEL, event.accessLevel);
        values.put(CalendarContract.Events.AVAILABILITY, event.availability);
        values.put(CalendarContract.Events.GUESTS_CAN_MODIFY, 1);
        return values;
    }

    private void addReminder(ReminderData data, long eventId)
    {
        ContentResolver cr = m_context.getContentResolver();
        ContentValues values = new ContentValues();
        values.put(CalendarContract.Reminders.MINUTES, data.minutes);
        values.put(CalendarContract.Reminders.METHOD, data.method);
        values.put(CalendarContract.Reminders.EVENT_ID, eventId);
        cr.insert(CalendarContract.ExtendedProperties.CONTENT_URI, values);
    }

    private void addAttendee(AttendeeData data, long eventId)
    {
        ContentResolver cr = m_context.getContentResolver();
        ContentValues values = new ContentValues();
        values.put(CalendarContract.Attendees.ATTENDEE_NAME, data.name);
        values.put(CalendarContract.Attendees.ATTENDEE_EMAIL, data.email);
        values.put(CalendarContract.Attendees.ATTENDEE_RELATIONSHIP, data.relationship);
        values.put(CalendarContract.Attendees.ATTENDEE_TYPE, data.type);
        values.put(CalendarContract.Attendees.ATTENDEE_STATUS, data.status);
        values.put(CalendarContract.Attendees.EVENT_ID, eventId);
        cr.insert(CalendarContract.Attendees.CONTENT_URI, values);
    }

    private void addExtendedProperty(ExtendedPropertyData data, long eventId)
    {
        ContentResolver cr = m_context.getContentResolver();
        ContentValues values = new ContentValues();
        values.put(CalendarContract.ExtendedProperties.NAME, data.name);
        values.put(CalendarContract.ExtendedProperties.VALUE, data.value);
        values.put(CalendarContract.ExtendedProperties.EVENT_ID, eventId);
        cr.insert(CalendarContract.ExtendedProperties.CONTENT_URI, values);
    }

    private void deleteReminders(long eventId)
    {
        ContentResolver cr = m_context.getContentResolver();
        String reminderSelection = "(( " + CalendarContract.Reminders.EVENT_ID + " == " + eventId + " ))";
        cr.delete(CalendarContract.Reminders.CONTENT_URI, reminderSelection, null);
    }

    private void deleteAttendees(long eventId)
    {
        ContentResolver cr = m_context.getContentResolver();
        String attendeeSelection = "(( " + CalendarContract.Attendees.EVENT_ID + " == " + eventId + " ))";
        cr.delete(CalendarContract.Attendees.CONTENT_URI, attendeeSelection, null);
    }

    private android.content.Context m_context;
    private long m_id;
}
