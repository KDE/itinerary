/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_URI_H
#define KANDROIDEXTRAS_URI_H

class QAndroidJniObject;
class QUrl;

namespace KAndroidExtras {

/** Conversion methods for android.net.Uri. */
namespace Uri
{
    /** Create an android.net.Uri from a QUrl. */
    QAndroidJniObject fromUrl(const QUrl &url);

    /** Convert a android.net.Uri to a QUrl. */
    QUrl toUrl(const QAndroidJniObject &uri);
}

}

#endif // KANDROIDEXTRAS_URI_H
