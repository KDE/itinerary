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
