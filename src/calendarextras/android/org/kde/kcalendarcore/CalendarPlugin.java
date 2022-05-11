
/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.kcalendarcore;

import android.content.*;
import android.database.*;
import android.provider.*;

public class CalendarPlugin
{
    public CalendarPlugin(android.content.Context context)
    {
        m_context = context;
    }

    private static final String[] CALENDAR_PROJECTION = new String[] {
        CalendarContract.Calendars._ID,
        CalendarContract.Calendars.CALENDAR_DISPLAY_NAME,
        CalendarContract.Calendars.CALENDAR_ACCESS_LEVEL,
        CalendarContract.Calendars.CALENDAR_COLOR,
        CalendarContract.Calendars.CALENDAR_TIME_ZONE
    };

    public CalendarData[] getCalendars()
    {
        ContentResolver cr = m_context.getContentResolver();
        String calSelection = "(( " + CalendarContract.Calendars.VISIBLE + " == 1 ) AND ( "
            + CalendarContract.Calendars.CALENDAR_ACCESS_LEVEL + " != " + CalendarContract.Calendars.CAL_ACCESS_NONE + " ))";
        Cursor cur = cr.query(CalendarContract.Calendars.CONTENT_URI, CALENDAR_PROJECTION, calSelection, null, null);

        CalendarData result[] = new CalendarData[cur.getCount()];
        while (cur.moveToNext()) {
            CalendarData data = new CalendarData();
            data.id = cur.getLong(0);
            data.displayName = cur.getString(1);
            data.accessLevel = cur.getInt(2);
            data.color = cur.getInt(3);
            data.timezone = cur.getString(4);
            result[cur.getPosition()] = data;
        }

        return result;
    }

    private android.content.Context m_context;
}
