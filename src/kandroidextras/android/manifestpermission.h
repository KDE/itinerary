/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_MANIFESTPERMISSIONM_H
#define KANDROIDEXTRAS_MANIFESTPERMISSIONM_H

#include <KAndroidExtras/AndroidTypes>
#include <KAndroidExtras/JniProperty>

namespace KAndroidExtras {

/**
 * Access to manifest permissions.
 * @see https://developer.android.com/reference/android/Manifest.permission
 */
class ManifestPermission : Jni::Wrapper<android::Manifest_permission>
{
public:
    JNI_CONSTANT(java::lang::String, READ_CALENDAR)
    JNI_CONSTANT(java::lang::String, READ_EXTERNAL_STORAGE)
    JNI_CONSTANT(java::lang::String, WRITE_EXTERNAL_STORAGE)
};

}

#endif // KANDROIDEXTRAS_MANIFESTPERMISSIONM_H
