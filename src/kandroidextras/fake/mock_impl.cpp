/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "QAndroidJniObject"

int JNIEnv::m_arrayLength = 0;
QStringList QAndroidJniObject::m_staticProtocol;
