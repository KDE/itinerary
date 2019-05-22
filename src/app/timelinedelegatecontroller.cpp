/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "timelinedelegatecontroller.h"

#include "reservationmanager.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/SortUtil>

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QTimer>

QTimer* TimelineDelegateController::s_currentTimer = nullptr;
int TimelineDelegateController::s_progressRefCount = 0;
QTimer* TimelineDelegateController::s_progressTimer = nullptr;

using namespace KItinerary;

TimelineDelegateController::TimelineDelegateController(QObject *parent)
    : QObject(parent)
{
    if (!s_currentTimer) {
        s_currentTimer = new QTimer(QCoreApplication::instance());
        s_currentTimer->setSingleShot(true);
        s_currentTimer->setTimerType(Qt::VeryCoarseTimer);
    }
    connect(s_currentTimer, &QTimer::timeout, this, [this]() { checkForUpdate(m_batchId); });
}

TimelineDelegateController::~TimelineDelegateController() = default;

QObject* TimelineDelegateController::reservationManager() const
{
    return m_resMgr;
}

void TimelineDelegateController::setReservationManager(QObject *resMgr)
{
    if (m_resMgr == resMgr)
        return;

    m_resMgr = qobject_cast<ReservationManager*>(resMgr);
    emit setupChanged();

    connect(m_resMgr, &ReservationManager::batchChanged, this, &TimelineDelegateController::checkForUpdate);
    connect(m_resMgr, &ReservationManager::batchContentChanged, this, &TimelineDelegateController::checkForUpdate);

    checkForUpdate(m_batchId);
}

QString TimelineDelegateController::batchId() const
{
    return m_batchId;
}

void TimelineDelegateController::setBatchId(const QString &batchId)
{
    if (m_batchId == batchId)
        return;

    m_batchId = batchId;
    emit contentChanged();
    checkForUpdate(batchId);
}

bool TimelineDelegateController::isCurrent() const
{
    return m_isCurrent;
}

void TimelineDelegateController::setCurrent(bool current, const QVariant &res)
{
    if (current == m_isCurrent) {
        return;
    }

    m_isCurrent = current;
    emit currentChanged();

    if (!LocationUtil::isLocationChange(res)) {
        return;
    }

    if (!s_progressTimer && m_isCurrent) {
        s_progressTimer = new QTimer(QCoreApplication::instance());
        s_progressTimer->setInterval(std::chrono::minutes(1));
        s_progressTimer->setTimerType(Qt::VeryCoarseTimer);
        s_progressTimer->setSingleShot(false);
    }

    if (m_isCurrent) {
        connect(s_progressTimer, &QTimer::timeout, this, &TimelineDelegateController::progressChanged);
        if (s_progressRefCount++ == 0) {
            s_progressTimer->start();
        }
    } else {
        disconnect(s_progressTimer, &QTimer::timeout, this, &TimelineDelegateController::progressChanged);
        if (--s_progressRefCount == 0) {
            s_progressTimer->stop();
        }
    }
}

float TimelineDelegateController::progress() const
{
    if (!m_resMgr || m_batchId.isEmpty() || !m_isCurrent) {
        return 0.0f;
    }

    const auto res = m_resMgr->reservation(m_batchId);
    // TODO this needs to consider live data
    const auto startTime = SortUtil::startDateTime(res);
    const auto endTime = SortUtil::endtDateTime(res);

    const auto tripLength = startTime.secsTo(endTime);
    if (tripLength <= 0) {
        return 0.0f;
    }
    const auto progress = startTime.secsTo(QDateTime::currentDateTime());

    return std::min(std::max(0.0f, (float)progress / (float)tripLength), 1.0f);
}

void TimelineDelegateController::checkForUpdate(const QString& batchId)
{
    if (!m_resMgr || m_batchId.isEmpty()) {
        setCurrent(false);
        return;
    }
    if (batchId != m_batchId) {
        return;
    }

    const auto res = m_resMgr->reservation(batchId);
    const auto now = QDateTime::currentDateTime();
    // TODO this needs to consider live data and boarding times
    const auto startTime = SortUtil::startDateTime(res);
    const auto endTime = SortUtil::endtDateTime(res);

    setCurrent(startTime < now && now < endTime, res);

    if (now < startTime) {
        scheduleNextUpdate(std::chrono::seconds(now.secsTo(startTime) + 1));
    } else if (now < endTime) {
        scheduleNextUpdate(std::chrono::seconds(now.secsTo(endTime) + 1));
    }
}

void TimelineDelegateController::scheduleNextUpdate(std::chrono::milliseconds ms)
{
    if (s_currentTimer->isActive() && s_currentTimer->remainingTimeAsDuration() < ms) {
        return;
    }
    s_currentTimer->start(ms);
}
