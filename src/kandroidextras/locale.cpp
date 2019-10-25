/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

    return QAndroidJniObject("java.util.Locale", Jni::signature<void(java::lang::String, java::lang::String, java::lang::String)>(),
        lang.object(), country.object(), script.object());
}

QAndroidJniObject Locale::current()
{
    return fromLocale(QLocale());
}
