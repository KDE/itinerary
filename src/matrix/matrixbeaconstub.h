// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef MATRIXBEACONSTUB_H
#define MATRIXBEACONSTUB_H

#include <QObject>
#include <QVariant>
#include <qqmlregistration.h>

class MatrixBeaconStub : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(MatrixBeacon)

    Q_PROPERTY(QVariant connection MEMBER m_connection)
    QVariant m_connection;
};

#endif
