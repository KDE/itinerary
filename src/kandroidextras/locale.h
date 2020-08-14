/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_LOCALE_H
#define KANDROIDEXTRAS_LOCALE_H

class QAndroidJniObject;
class QLocale;

namespace KAndroidExtras {

/** Conversion methods between java.util.Locale and QLocale. */
namespace Locale
{
    /** Create an java.util.Locale object from a QLocale. */
    QAndroidJniObject fromLocale(const QLocale &locale);

    /** Create an java.util.Locale object for the current QLocale. */
    QAndroidJniObject current();
}

}

#endif // KANDROIDEXTRAS_LOCALE_H
