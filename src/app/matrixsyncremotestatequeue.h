/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MATRIXSYNCREMOTESTATEQUEUE_H
#define MATRIXSYNCREMOTESTATEQUEUE_H

#include "matrixsyncstateevent.h"

#include <QObject>

#include <deque>
#include <unordered_set>

class MatrixSyncStateEvent;

namespace Quotient {
class StateEvent;
}

class QJsonObject;

/** Pending remote changes awaiting file download and local application.
 *  TODO could be made persistent for extra robustness
 */
class MatrixSyncRemoteStateQueue : public QObject
{
    Q_OBJECT
public:
    explicit MatrixSyncRemoteStateQueue(QObject *parent = nullptr);
    ~MatrixSyncRemoteStateQueue();

    /** Enqueue a new state event change. */
    void append(const Quotient::StateEvent &state, const QString &roomId);

    /** Return the results of a file download. */
    void setFileName(const QString &fileName);
    void downloadFailed();

    /** Record an event id we already know, typically because we created it ourselves. */
    void addKnownEventId(const QString &eventId);

Q_SIGNALS:
    /** Next state event needs a download of an external file. */
    void downloadFile(const MatrixSyncStateEvent &state);

    /** State events ready for being applied locally. */
    void reservationEvent(const MatrixSyncStateEvent &state);
    void liveDataEvent(const MatrixSyncStateEvent &state);
    void transferEvent(const MatrixSyncStateEvent &state);
    void documentEvent(const MatrixSyncStateEvent &state);
    void pkPassEvent(const MatrixSyncStateEvent &state);

private:
    void dispatchNext();

    std::deque<MatrixSyncStateEvent> m_pendingChanges;
    std::unordered_set<QString> m_seenEventIds;
};

#endif
