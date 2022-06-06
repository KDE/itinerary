/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_CONTEXT_H
#define KANDROIDEXTRAS_CONTEXT_H

#include "kandroidextras_export.h"

#include <qglobal.h>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QAndroidJniObject;
#else
class QJniObject;
using QAndroidJniObject = QJniObject;
#endif

namespace KAndroidExtras {

/** Methods around android.content.Context. */
namespace Context
{
    KANDROIDEXTRAS_EXPORT QAndroidJniObject getPackageName();
}

}

#endif // KANDROIDEXTRAS_CONTEXT_H
