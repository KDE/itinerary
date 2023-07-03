/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.kcalendarcore;

/** Struct for transferring calendar data over JNI. */
public class CalendarData
{
    public long id;
    public java.lang.String displayName;
    public int accessLevel;
    public int color;
    public java.lang.String timezone;
    public java.lang.String owner;
}
