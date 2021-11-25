/*
    SPDX-FileCopyrightText: 2019-2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNICOMMON_H
#define KANDROIDEXTRAS_JNICOMMON_H

namespace KAndroidExtras {

namespace Jni {

/** Wrapper type for array return values (which we cannot specify using [] syntax). */
template <typename T> struct Array {};

}

}

#endif
