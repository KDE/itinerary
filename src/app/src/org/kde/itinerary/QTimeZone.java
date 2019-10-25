/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
