/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TRANSFERDELEGATECONTROLLER_H
#define TRANSFERDELEGATECONTROLLER_H

#include "transfer.h"

#include <QObject>
#include <QTimer>

/** Transfer delegate logic. */
class TransferDelegateController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Transfer transfer READ transfer WRITE setTransfer NOTIFY transferChanged)
    Q_PROPERTY(bool isCurrent READ isCurrent NOTIFY updated)
    Q_PROPERTY(float progress READ progress NOTIFY updated)

public:
    explicit TransferDelegateController(QObject *parent = nullptr);
    ~TransferDelegateController();

    Transfer transfer() const;
    void setTransfer(const Transfer &transfer);

    bool isCurrent() const;
    float progress() const;

Q_SIGNALS:
    void transferChanged();
    void updated();

private:
    void scheduleTimer();

    Transfer m_transfer;
    QTimer m_updateTrigger;
};

#endif // TRANSFERDELEGATECONTROLLER_H
