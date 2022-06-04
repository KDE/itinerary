/*
    SPDX-FileCopyrightText: 2020-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_MOCK_QANDROIDJNIOBJECT_H
#define KANDROIDEXTRAS_MOCK_QANDROIDJNIOBJECT_H

#include "mock_jniobject.h"

/** Mock object for QJniObject outside of Android, for automated testing. */
class KANDROIDEXTRAS_EXPORT QAndroidJniObject : public KAndroidExtras::Internal::MockJniObject<QAndroidJniObject> {
public:
    using MockJniObject<QAndroidJniObject>::MockJniObject;
};

Q_DECLARE_METATYPE(QAndroidJniObject)

#endif
