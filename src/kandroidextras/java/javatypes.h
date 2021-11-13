/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JAVATYPES_H
#define KANDROIDEXTRAS_JAVATYPES_H

#include <KAndroidExtras/JniTypes>
#include <KAndroidExtras/JniTypeTraits>

namespace KAndroidExtras {

JNI_TYPE(java, io, File)
JNI_TYPE(java, lang, String)
JNI_TYPE(java, util, Locale)

namespace Jni {
template <> struct converter<QString> {
    typedef java::lang::String type;
    static inline QAndroidJniObject convert(const QString &value) { return QAndroidJniObject::fromString(value); }
};
template <> struct converter<java::lang::String> {
    typedef QString type;
    static inline QString convert(const QAndroidJniObject &value) { return value.toString(); }
};
}

}

#endif // KANDROIDEXTRAS_JAVATYPES_H

