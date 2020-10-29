/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "locale.h"
#include "jnisignature.h"
#include "jnitypes.h"

#include <QAndroidJniObject>
#include <QLocale>

using namespace KAndroidExtras;

QAndroidJniObject Locale::fromLocale(const QLocale &locale)
{
    auto lang = QAndroidJniObject::fromString(QLocale::languageToString(locale.language()));
    auto country = QAndroidJniObject::fromString(QLocale::countryToString(locale.country()));
    auto script = QAndroidJniObject::fromString(QLocale::scriptToString(locale.script()));

    return QAndroidJniObject(Jni::typeName<java::util::Locale>(), Jni::signature<void(java::lang::String, java::lang::String, java::lang::String)>(),
        lang.object(), country.object(), script.object());
}

QAndroidJniObject Locale::current()
{
    return fromLocale(QLocale());
}
