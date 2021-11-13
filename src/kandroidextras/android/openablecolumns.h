/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_OPENABLECOLUMNS_H
#define KANDROIDEXTRAS_OPENABLECOLUMNS_H

#include <KAndroidExtras/AndroidTypes>
#include <KAndroidExtras/JniProperty>
#include <KAndroidExtras/JavaTypes>

namespace KAndroidExtras {

/**
 * Constants for ContentResolver queries.
 * @see https://developer.android.com/reference/android/provider/OpenableColumns
 */
class OpenableColumns : android::provider::OpenableColumns
{
    JNI_OBJECT(OpenableColumns)
public:
    JNI_CONSTANT(java::lang::String, DISPLAY_NAME)
    JNI_CONSTANT(java::lang::String, SIZE)
};

}

#endif // KANDROIDEXTRAS_OPENABLECOLUMNS_H

