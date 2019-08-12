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

#ifndef KANDROIDEXTRAS_CONTENTRESOLVER_H
#define KANDROIDEXTRAS_CONTENTRESOLVER_H

class QAndroidJniObject;
class QString;
class QUrl;

namespace KAndroidExtras {

/** Methods for working with Android's ContentResolver. */
namespace ContentResolver
{
    /** Get the JNI content resolver. */
    QAndroidJniObject get();

    /** Returns the mime type of the given content: URL.
     * @see Android ContentResolver.getType
     */
    QString mimeType(const QUrl &url);

    /** File name of a file provided by a content: URL. */
    QString fileName(const QUrl &url);
};

}

#endif // KANDROIDEXTRAS_CONTENTRESOLVER_H
