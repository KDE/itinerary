// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <Quotient/keyverificationsession.h>
#include <qqmlregistration.h>
#include "config-matrix.h"
#include <QObject>

#if HAVE_MATRIX
class KeyVerificationSessionForeign : public QObject
{
    Q_OBJECT
    QML_UNCREATABLE("")
    QML_NAMED_ELEMENT(KeyVerificationSession)
    QML_FOREIGN(Quotient::KeyVerificationSession)
};
#else
class MatrixBeaconStub : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(MatrixBeacon)
    Q_PROPERTY(QVariant connection MEMBER m_connection)
    QVariant m_connection;
};
#endif
