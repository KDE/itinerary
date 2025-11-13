/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TRANSFERDELEGATECONTROLLER_H
#define TRANSFERDELEGATECONTROLLER_H

#include "transfer.h"

#include <QChronoTimer>
#include <QObject>
#include <qqmlregistration.h>

/** Transfer delegate logic. */
class TransferDelegateController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(Transfer transfer READ transfer WRITE setTransfer NOTIFY transferChanged)
    Q_PROPERTY(bool isCurrent READ isCurrent NOTIFY updated)
    Q_PROPERTY(float progress READ progress NOTIFY updated)

public:
    explicit TransferDelegateController(QObject *parent = nullptr);
    ~TransferDelegateController();

    [[nodiscard]] Transfer transfer() const;
    void setTransfer(const Transfer &transfer);

    [[nodiscard]] bool isCurrent() const;
    [[nodiscard]] float progress() const;

Q_SIGNALS:
    void transferChanged();
    void updated();

private:
    void scheduleTimer();

    Transfer m_transfer;
    QChronoTimer m_updateTrigger;
};

#endif // TRANSFERDELEGATECONTROLLER_H
