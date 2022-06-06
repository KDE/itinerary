/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_CONTENTRESOLVER_H
#define KANDROIDEXTRAS_CONTENTRESOLVER_H

#include "kandroidextras_export.h"

#include <qglobal.h>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QAndroidJniObject;
#else
class QJniObject;
using QAndroidJniObject = QJniObject;
#endif
class QString;
class QUrl;

namespace KAndroidExtras {

/** Methods for working with Android's ContentResolver. */
namespace ContentResolver
{
    /** Get the JNI content resolver. */
    KANDROIDEXTRAS_EXPORT QAndroidJniObject get();

    /** Returns the mime type of the given content: URL.
     * @see Android ContentResolver.getType
     */
    KANDROIDEXTRAS_EXPORT QString mimeType(const QUrl &url);

    /** File name of a file provided by a content: URL. */
    KANDROIDEXTRAS_EXPORT QString fileName(const QUrl &url);
}

}

#endif // KANDROIDEXTRAS_CONTENTRESOLVER_H
