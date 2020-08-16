/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.itinerary;

import android.icu.text.TimeZoneFormat;
import android.icu.util.TimeZone;
import android.os.Build;

import java.lang.String;
import java.util.Locale;
import android.util.Log;

public class QTimeZone
{
    public static String abbreviation(String tzid, long date, Locale locale, boolean isDaylightTime)
    {
        // ICU timezone formatting gives the best result, but is only available starting at API level 24
        if (Build.VERSION.SDK_INT >= 24) {
            TimeZoneFormat format = TimeZoneFormat.getInstance(locale);
            TimeZone tz = TimeZone.getTimeZone(tzid);
            return format.format(TimeZoneFormat.Style.SPECIFIC_SHORT, tz, date);
        }

        // fallback to java.util.TimeZone, this only gives us GMT offsets
        java.util.TimeZone tz = java.util.TimeZone.getTimeZone(tzid);
        return tz.getDisplayName(isDaylightTime, java.util.TimeZone.SHORT, locale);
    }
}
