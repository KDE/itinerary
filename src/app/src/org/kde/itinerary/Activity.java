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

    public String receiveContent(Uri uri)
    {
        ContentResolver resolver = getContentResolver();
        File tempFile = new File(getCacheDir(), "content"); // TODO use random name

        try {
            InputStream is = resolver.openInputStream(uri);

            tempFile.createNewFile();
            FileOutputStream os = new FileOutputStream(tempFile);

            while (true) {
                byte[] buffer = new byte[4096];
                int size = is.read(buffer, 0, 4096);
                if (size < 0) {
                    break;
                }
                os.write(buffer, 0, size);
            }
            os.close();
        } catch (Exception e) {
            Log.e(TAG, e.toString());
            return "";
        }

        return tempFile.toString();
    }

    public void launchViewIntentFromUri(String uri)
    {
        Uri mapIntentUri = Uri.parse(uri);
        Intent mapIntent = new Intent(Intent.ACTION_VIEW, mapIntentUri);
        startActivity(mapIntent);
    }

    public native void importReservation(String data);

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
                if (propCursor.getString(0) == null || !propCursor.getString(0).equals("unknown-property") || propCursor.getString(1) == null) {
                    continue;
                }
                ByteArrayInputStream bis = new ByteArrayInputStream(android.util.Base64.decode(propCursor.getString(1), android.util.Base64.NO_WRAP));
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

    public void maxBrightness() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                WindowManager.LayoutParams layout = getWindow().getAttributes();
                layout.screenBrightness = 1F;
                getWindow().setAttributes(layout);
            }
        });
    }
}
