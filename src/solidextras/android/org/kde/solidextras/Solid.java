/*
    SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.solidextras;

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
}
