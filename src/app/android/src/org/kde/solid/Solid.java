/*
    Copyright (C) 2018 Nicolas Fella <nicolas.fella@gmx.de>

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
        activity.runOnUiThread(() -> {
            activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        });
    }

    public static void setLockInhibitionOff(Activity activity) {
        activity.runOnUiThread(() -> {
            activity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        });
    }
}
