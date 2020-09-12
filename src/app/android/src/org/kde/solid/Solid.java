/*
    SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.solid;

import android.app.Activity;
import android.view.WindowManager;

public class Solid
{
    public static float getBrightness(Activity activity) {
        return activity.getWindow().getAttributes().screenBrightness;
    }

    public static void setBrightness(Activity activity, final float brightness) {
        activity.runOnUiThread(() -> {
            WindowManager.LayoutParams layout = activity.getWindow().getAttributes();
            layout.screenBrightness = brightness;
            activity.getWindow().setAttributes(layout);
        });
    }

    public static void setLockInhibitionOn(Activity activity) {
        activity.runOnUiThread(() -> activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON));
    }

    public static void setLockInhibitionOff(Activity activity) {
        activity.runOnUiThread(() -> activity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON));
    }
}
