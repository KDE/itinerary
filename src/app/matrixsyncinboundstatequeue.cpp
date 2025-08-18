/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixsyncinboundstatequeue.h"

#include "logging.h"

#include <Quotient/events/stateevent.h>

using namespace Qt::Literals;

MatrixSyncInboundStateQueue::MatrixSyncInboundStateQueue(QObject *parent) :
    QObject(parent)
{
}

MatrixSyncInboundStateQueue::~MatrixSyncInboundStateQueue() = default;

void MatrixSyncInboundStateQueue::append(const Quotient::StateEvent &state, const QString &roomId)
{
    auto ev = MatrixSyncStateEvent::fromQuotient(state, roomId);
    if (!ev || m_seenEventIds.contains(state.id())) {
        return;
    }

    qCDebug(Log) << "Got remote state event" << ev->type() << ev->stateKey() << ev->roomId();
    // event compression when we get a larger reply in one go
    for (auto it = m_pendingChanges.begin(); it != m_pendingChanges.end(); ++it) {
        if ((*it).type() != ev->type() && (*it).stateKey() == ev->stateKey()) {
            continue;
        }
        if (it == m_pendingChanges.begin() && (*it).needsDownload()) {
            // currently being downloaded, so don't interfere with that
            continue;
        }
        (*it) = std::move(*ev);
        qCDebug(Log) << "... replacing already pending state event";
        return;
    }

    m_pendingChanges.push_back(std::move(*ev));
    Q_EMIT queueChanged();

    if (m_pendingChanges.size() == 1) {
        dispatchNext();
    }
}

void MatrixSyncInboundStateQueue::setFileName(const QString &fileName)
{
    if (m_pendingChanges.empty()) {
        return;
    }

    auto &ev = m_pendingChanges.front();
    qCDebug(Log) << "Download complete for state event" << ev.type() << ev.stateKey() << ev.url() << fileName;
    ev.setFileName(fileName);
    if (!ev.needsDownload()) {
        dispatchNext();
    }
}

void MatrixSyncInboundStateQueue::downloadFailed()
{
    if (m_pendingChanges.empty()) {
        return;
    }
    qCWarning(Log) << "Discarding remote state change due to failed download" << m_pendingChanges.front().type() << m_pendingChanges.front().stateKey();
    m_pendingChanges.pop_front();
    Q_EMIT queueChanged();
    dispatchNext();
}

void MatrixSyncInboundStateQueue::addKnownEventId(const QString &eventId)
{
    m_seenEventIds.insert(eventId);
}

void MatrixSyncInboundStateQueue::dispatchNext()
{
    if (m_pendingChanges.empty()) {
        return;
    }

    const auto &ev = m_pendingChanges.front();
    if (ev.needsDownload()) {
        qCDebug(Log) << "Downloading" << ev.type() << ev.stateKey() << ev.url();
        Q_EMIT downloadFile(ev);
        return;
    }

    qCDebug(Log) << "Dispatching remote state event" << ev.type() << ev.stateKey();
    if (ev.type() == MatrixSync::ReservationEventType) {
        Q_EMIT reservationEvent(ev);
    } else if (ev.type() == MatrixSync::LiveDataEventType) {
        Q_EMIT liveDataEvent(ev);
    } else if (ev.type() == MatrixSync::TransferEventType) {
        Q_EMIT transferEvent(ev);
    } else if (ev.type() == MatrixSync::DocumentEventType) {
        Q_EMIT documentEvent(ev);
    } else if (ev.type() == MatrixSync::PkPassEventType) {
        Q_EMIT pkPassEvent(ev);
    } else {
        qCWarning(Log) << "Unhandled event type:" << ev.type();
    }

    m_pendingChanges.pop_front();
    Q_EMIT queueChanged();
    dispatchNext();
}

#include "moc_matrixsyncinboundstatequeue.cpp"
