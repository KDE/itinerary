/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "intent.h"
#include "uri.h"

#include <KAndroidExtras/JniArray>
#include <KAndroidExtras/JniSignature>

#include <QAndroidJniEnvironment>
#include <QUrl>

using namespace KAndroidExtras;

Intent::Intent()
{
    setJniHandle(QAndroidJniObject(Jni::typeName<android::content::Intent>()));
}

Intent::~Intent() = default;

Intent::operator QAndroidJniObject() const
{
    return jniHandle();
}

template <typename T>
QAndroidJniObject Intent::getObjectExtra(const char *methodName, const QAndroidJniObject &name) const
{
    return jniHandle().callObjectMethod(methodName, Jni::signature<T(java::lang::String)>(), name.object());
}

QString Intent::getStringExtra(const QAndroidJniObject &name) const
{
    return getObjectExtra<java::lang::String>("getStringExtra", name).toString();
}

QStringList Intent::getStringArrayExtra(const QAndroidJniObject &name) const
{
    const auto extra = getObjectExtra<Jni::Array<java::lang::String>>("getStringArrayExtra", name);
    return Jni::fromArray<QStringList>(extra);
}
