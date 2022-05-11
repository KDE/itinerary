/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.kcalendarcore;

/** Struct for transferring event extended property data over JNI. */
public class ExtendedPropertyData
{
    public ExtendedPropertyData() {}

    public java.lang.String name;
    public java.lang.String value;
}
