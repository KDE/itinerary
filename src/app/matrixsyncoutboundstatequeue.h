/*
    SPDX-FileCopyrightText: ⓒ 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MATRIXSYNCOUTBOUNDSTATEQUEUE_H
#define MATRIXSYNCOUTBOUNDSTATEQUEUE_H

#include "transfer.h"

#include <QObject>

#include <deque>

class QJsonObject;

/** Pending local changes awaiting upload. */
class MatrixSyncOutboundStateQueue : public QObject
{
    Q_OBJECT
public:
    explicit MatrixSyncOutboundStateQueue(QObject *parent = nullptr);
    ~MatrixSyncOutboundStateQueue();

    enum ChangeType {
        BatchChange,
        BatchRemove,
        LiveDataChange,
        TransferChange,
        DocumentAdd,
        PkPassChange,
        TripGroupAdded,
        TripGroupChanged
    };
    Q_ENUM(ChangeType)

    /** Enqueue a new change.
     *  Will be discarded when change tracking is suspended.
     */
    void append(ChangeType type, const QString &id, const QString &context = {});
    /** Current change successfully applied. */
    void replayNext();
    /** Retry the current change, if any. */
    void retry();

    /** Suspend recording changes.
     *  Do this while applying remote changes localls to avoid recursion.
     *  New changes will be ignored while suspended.
     */
    void suspend();
    [[nodiscard]] bool isSuspended() const;
    /** Resume recording. */
    void resume();

    [[nodiscard]] bool isEmpty() const { return m_pendingChanges.empty(); }
    [[nodiscard]] auto size() const { return m_pendingChanges.size(); }

    // for testing only
    [[nodiscard]] static QString path();

Q_SIGNALS:
    void batchChanged(const QString &batchId, const QString &tgId);
    void batchRemoved(const QString &batchId, const QString &tgId);
    void liveDataChanged(const QString &batchId);
    void transferChanged(const QString &batchId, Transfer::Alignment alignment);
    void documentAdded(const QString &docId, const QString &tgId);
    void pkPassChanged(const QString &pkPassId, const QString &tgId);
    void tripGroupAdded(const QString &tgId);
    void tripGroupChanged(const QString &tgId);

    /** Any change to the queue. */
    void queueChanged();

private:
    void store();
    void load();
    void doReplayNext();

    class StateChange {
    public:
        [[nodiscard]] bool operator==(const StateChange &other) const;

        [[nodiscard]] static QJsonObject toJson(const StateChange &change);
        [[nodiscard]] static std::optional<StateChange> fromJson(const QJsonObject &obj);

        MatrixSyncOutboundStateQueue::ChangeType type;
        QString id;
        QString context;
    };

    std::deque<StateChange> m_pendingChanges;
    bool m_suspended = false;
};

/** RAII type to suspend local change recording. */
class MatrixSyncLocalChangeLock
{
public:
    explicit MatrixSyncLocalChangeLock(MatrixSyncOutboundStateQueue *queue)
    {
        if (queue && !queue->isSuspended()) {
            m_queue = queue;
            m_queue->suspend();
        }
    }

    ~MatrixSyncLocalChangeLock()
    {
        if (m_queue) {
            m_queue->resume();
        }
    }

private:
    MatrixSyncOutboundStateQueue *m_queue = nullptr;
};

#endif
