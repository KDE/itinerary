/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_ACTIVITY_H
#define KANDROIDEXTRAS_ACTIVITY_H

namespace KAndroidExtras {

class Intent;

/** Methods around android.app.Activity. */
namespace Activity
{
    /** Returns the Intent that started the activity. */
    Intent getIntent();

    /** Same as QtAndroid::startActivity(), but with exception handling. */
    bool startActivity(const Intent &intent, int receiverRequestCode); // TODO add callback arg
}

}

#endif // KANDROIDEXTRAS_ACTIVITY_H
