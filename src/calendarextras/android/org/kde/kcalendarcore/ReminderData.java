/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.kcalendarcore;

/** Struct for transferring event reminder data over JNI. */
public class ReminderData
{
    public ReminderData() {}

    public int minutes;
    public int method;
}
