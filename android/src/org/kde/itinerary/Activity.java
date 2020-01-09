/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

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

import org.qtproject.qt5.android.bindings.QtActivity;

import net.fortuna.ical4j.model.property.XProperty;

import androidx.core.content.FileProvider;

import android.content.ActivityNotFoundException;
import android.content.ContentResolver;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.provider.CalendarContract;
import android.util.Log;
import android.view.WindowManager;

import java.io.*;
import java.util.*;


public class Activity extends QtActivity
{
    private static final String TAG = "org.kde.itinerary";

    public native void importReservation(String data);
    public native void importDavDroidJson(String data);
    public native void importFromIntent(Intent data);

    /** Check the calendar for with JSON-LD data.
     *  This assumes the custom property serialization format used by DavDroid.
     */
    public void checkCalendar()
    {
        Calendar startTime = Calendar.getInstance();
        startTime.add(Calendar.DAY_OF_YEAR, -5);
        Calendar endTime = Calendar.getInstance();
        endTime.add(Calendar.MONTH, 6);

        String[] eventColumns = new String[] { "uid2445", "title", "_id" };
        String[] propColumns = new String[] { "name", "value" };

        String eventSelection = "(( " + CalendarContract.Events.DTSTART + " >= " + startTime.getTimeInMillis()
            + " ) AND ( " + CalendarContract.Events.DTSTART + " <= " + endTime.getTimeInMillis() + " ))";

        Cursor cursor = getContentResolver().query(CalendarContract.Events.CONTENT_URI, eventColumns, eventSelection, null, null);
        while (cursor.moveToNext()) {
            if (cursor.getString(0) == null || !cursor.getString(0).startsWith("KIT-")) {
                continue;
            }
            Log.i(TAG, cursor.getString(1));

            String propSelection = "(event_id == " + cursor.getInt(2) + ")";
            Cursor propCursor = getContentResolver().query(CalendarContract.ExtendedProperties.CONTENT_URI, propColumns, propSelection, null, null);
            while (propCursor.moveToNext()) {
                String propName = propCursor.getString(0);
                String propValue = propCursor.getString(1);
                if (propName == null || propValue == null) {
                    continue;
                }

                Log.i(TAG, propName);
                if (propName.equals("unknown-property.v2") || propName.contains("vnd.ical4android.unknown-property")) {
                    importDavDroidJson(propValue);

                // legacy, replaced by the above in Feb 2019, removing this eventually will allow us to remove the ical4j dependency
                } else if (propName.equals("unknown-property")) {
                    ByteArrayInputStream bis = new ByteArrayInputStream(android.util.Base64.decode(propValue, android.util.Base64.NO_WRAP));
                    try  {
                        ObjectInputStream ois = new ObjectInputStream(bis);
                        Object prop = ois.readObject();
                        if (prop instanceof XProperty) {
                            importReservation(((XProperty)prop).getValue());
                        }
                    } catch (Exception e) {
                        Log.i(TAG, e.toString());
                        continue;
                    }
                }
            }
        }
    }

    public void setBrightness(final float brightness) {
        runOnUiThread(() -> {
            WindowManager.LayoutParams layout = getWindow().getAttributes();
            layout.screenBrightness = brightness;
            getWindow().setAttributes(layout);
        });
    }

    public float getBrightness() {
        return getWindow().getAttributes().screenBrightness;
    }

    public void setLockInhibitionOn() {
        runOnUiThread(() -> getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON));
    }

    public void setLockInhibitionOff() {
        runOnUiThread(() -> getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON));
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        importFromIntent(intent);
    }

    public Uri openDocument(String filePath)
    {
        Log.i(TAG, filePath);
        File file = new File(filePath);
        return FileProvider.getUriForFile(this, "org.kde.itinerary.documentProvider", file);
    }
}
