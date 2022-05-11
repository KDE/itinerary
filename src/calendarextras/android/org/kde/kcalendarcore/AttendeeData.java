/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.kcalendarcore;

/** Struct for transferring event attendee data over JNI. */
public class AttendeeData
{
    public AttendeeData() {}

    public java.lang.String name;
    public java.lang.String email;
    int relationship;
    int type;
    int status;
}
