/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SOLIDEXTRAS_NETWORKSTATUS_H
#define SOLIDEXTRAS_NETWORKSTATUS_H

#include "solidextras_export.h"

#include <QObject>

namespace SolidExtras {

/** Basic information about the network status (connectivity, metering). */
class SOLIDEXTRAS_EXPORT NetworkStatus : public QObject
{
    Q_OBJECT
    Q_PROPERTY(State connectivity READ connectivity NOTIFY connectivityChanged)
    Q_PROPERTY(State metered READ metered NOTIFY meteredChanged)
public:
    enum State {
        Unknown,
        Yes,
        No
    };
    Q_ENUM(State)

    explicit NetworkStatus(QObject *parent = nullptr);
    ~NetworkStatus();

    State connectivity() const;
    State metered() const;

Q_SIGNALS:
    void connectivityChanged();
    void meteredChanged();
};

}

#endif // SOLIDEXTRAS_NETWORKSTATUS_H
