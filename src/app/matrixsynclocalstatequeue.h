/*
    SPDX-FileCopyrightText: ⓒ 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MATRIXSYNCLOCALSTATEQUEUE_H
#define MATRIXSYNCLOCALSTATEQUEUE_H

#include "transfer.h"

#include <QObject>

#include <deque>

class QJsonObject;

class MatrixSyncLocalStateChange;

/** Pending local changes awaiting upload. */
class MatrixSyncLocalStateQueue : public QObject
{
    Q_OBJECT
public:
    explicit MatrixSyncLocalStateQueue(QObject *parent = nullptr);
    ~MatrixSyncLocalStateQueue();

    enum ChangeType {
        BatchChange,
        BatchRemove,
        LiveDataChange,
        TransferChange,
        DocumentAdd,
        PkPassChange,
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
    [[nodiscard]] const MatrixSyncLocalStateChange& front() const { return m_pendingChanges.front(); }

    // for testing only
    [[nodiscard]] static QString path();

Q_SIGNALS:
    void batchChanged(const QString &batchId, const QString &tgId);
    void batchRemoved(const QString &batchId, const QString &tgId);
    void liveDataChanged(const QString &batchId);
    void transferChanged(const QString &batchId, Transfer::Alignment alignment);
    void documentAdded(const QString &docId, const QString &tgId);
    void pkPassChanged(const QString &pkPassId, const QString &tgId);
    // TODO trip group changes?

private:
    void store();
    void load();
    void doReplayNext();

    std::deque<MatrixSyncLocalStateChange> m_pendingChanges;
    bool m_suspended = false;
};

class MatrixSyncLocalStateChange {
public:
    [[nodiscard]] bool operator==(const MatrixSyncLocalStateChange &other) const;

    [[nodiscard]] static QJsonObject toJson(const MatrixSyncLocalStateChange &change);
    [[nodiscard]] static std::optional<MatrixSyncLocalStateChange> fromJson(const QJsonObject &obj);

    MatrixSyncLocalStateQueue::ChangeType type;
    QString id;
    QString context;
};

/** RAII type to suspend local change recording. */
class MatrixSyncLocalChangeLock
{
public:
    explicit MatrixSyncLocalChangeLock(MatrixSyncLocalStateQueue *queue)
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
    MatrixSyncLocalStateQueue *m_queue = nullptr;
};

#endif
