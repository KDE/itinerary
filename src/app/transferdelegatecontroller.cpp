/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "transferdelegatecontroller.h"

#include <QDebug>

TransferDelegateController::TransferDelegateController(QObject *parent)
    : QObject(parent)
{
    m_updateTrigger.setTimerType(Qt::VeryCoarseTimer);
    connect(&m_updateTrigger, &QTimer::timeout, this, &TransferDelegateController::updated);
    connect(&m_updateTrigger, &QTimer::timeout, this, &TransferDelegateController::scheduleTimer);
}

TransferDelegateController::~TransferDelegateController() = default;

Transfer TransferDelegateController::transfer() const
{
    return m_transfer;
}

void TransferDelegateController::setTransfer(const Transfer &transfer)
{
    m_transfer = transfer;
    Q_EMIT transferChanged();
    Q_EMIT updated();

    if (transfer.state() != Transfer::Selected) {
        m_updateTrigger.stop();
        return;
    }

    scheduleTimer();
}

bool TransferDelegateController::isCurrent() const
{
    const auto jny = m_transfer.journey();
    const auto now = QDateTime::currentDateTime();

    if ((jny.hasExpectedArrivalTime() && jny.expectedArrivalTime() < now) || jny.scheduledArrivalTime() < now) {
        return false;
    }
    if ((jny.departureDelay() < 0 && now < jny.expectedDepartureTime()) || now < jny.scheduledDepartureTime()) {
        return false;
    }

    return true;
}

float TransferDelegateController::progress() const
{
    const auto jny = m_transfer.journey();
    const auto startTime = jny.hasExpectedDepartureTime() ? jny.expectedDepartureTime() : jny.scheduledDepartureTime();
    const auto endTime = jny.hasExpectedArrivalTime() ? jny.expectedArrivalTime() : jny.scheduledArrivalTime();

    const auto tripLength = startTime.secsTo(endTime);
    if (tripLength <= 0) {
        return 0.0f;
    }
    const auto progress = startTime.secsTo(QDateTime::currentDateTime());

    return std::min(std::max(0.0f, (float)progress / (float)tripLength), 1.0f);
}

void TransferDelegateController::scheduleTimer()
{
    const auto jny = m_transfer.journey();
    const auto now = QDateTime::currentDateTime();

    if ((jny.hasExpectedArrivalTime() && jny.expectedArrivalTime() < now) || jny.scheduledArrivalTime() < now) { // already arrived
        m_updateTrigger.stop();
        return;
    }
    const auto depTime = jny.departureDelay() < 0 ? jny.expectedDepartureTime() : jny.scheduledDepartureTime();
    if (now < depTime) {
        m_updateTrigger.setInterval(now.secsTo(depTime) * 1000);
        m_updateTrigger.start();
        return;
    }

    m_updateTrigger.setInterval(std::chrono::minutes(1));
    m_updateTrigger.start();
}
