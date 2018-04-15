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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

package org.kde.itinerary;

import org.qtproject.qt5.android.bindings.QtActivity;

import android.content.ContentResolver;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;

import java.io.*;


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
}
