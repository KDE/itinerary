/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_URI_H
#define KANDROIDEXTRAS_URI_H

#include "kandroidextras_export.h"

class QAndroidJniObject;
class QUrl;

namespace KAndroidExtras {

/** Conversion methods for android.net.Uri. */
namespace Uri
{
    /** Create an android.net.Uri from a QUrl. */
    KANDROIDEXTRAS_EXPORT QAndroidJniObject fromUrl(const QUrl &url);

    /** Convert a android.net.Uri to a QUrl. */
    KANDROIDEXTRAS_EXPORT QUrl toUrl(const QAndroidJniObject &uri);
}

}

#endif // KANDROIDEXTRAS_URI_H
