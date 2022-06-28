/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.itinerary;

import org.qtproject.qt5.android.bindings.QtActivity;

import androidx.core.content.FileProvider;

import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.provider.CalendarContract;
import android.os.Bundle;
import android.util.Log;

import java.io.*;
import java.util.*;


public class Activity extends QtActivity
{
    private static final String TAG = "org.kde.itinerary";

    public native void importFromIntent(Intent data);

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        importFromIntent(intent);
    }

    public Uri openDocument(String filePath)
    {
        Log.i(TAG, filePath);
        File file = new File(filePath);
        return FileProvider.getUriForFile(this, "org.kde.itinerary.documentprovider", file);
    }

    /* Try to find attachment URLs from shared content from email applications.
     * - Fair Email adds content: URLs as EXTRA_STREAM, as a single Uri or an ArrayList<Uri>.
     */
    public String[] attachmentsForIntent(Intent intent)
    {
        try {
            Bundle extras = intent.getExtras();
            if (extras == null) {
                return null;
            }

            Object streamObj = extras.get(Intent.EXTRA_STREAM);
            if (streamObj == null) {
                return null;
            }

            Log.i(TAG, streamObj.getClass().toString());
            if (streamObj instanceof java.util.ArrayList) {
                ArrayList l = (ArrayList)streamObj;
                String[] result = new String[l.size()];
                for (int i = 0; i < l.size(); ++i) {
                    Object entry = l.get(i);
                    if (entry instanceof android.net.Uri) {
                        Uri uri = (Uri)entry;
                        result[i] = uri.toString();
                        continue;
                    }

                    Log.i(TAG, "unhandled array entry");
                    Log.i(TAG, entry.getClass().toString());
                    Log.i(TAG, entry.toString());
                }
                return result;
            }

            if (streamObj instanceof android.net.Uri) {
                Uri uri = (Uri)streamObj;
                String[] result = new String[1];
                result[0] = uri.toString();
                return result;
            }

            Log.i(TAG, "unhandled EXTRA_STREAM content type");
            Log.i(TAG, streamObj.getClass().toString());
            Log.i(TAG, streamObj.toString());
        } catch (java.lang.Exception e) {
            Log.i(TAG, e.toString());
        }
        return null;
    }
}
