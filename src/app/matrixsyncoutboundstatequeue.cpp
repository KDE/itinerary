/*
    SPDX-FileCopyrightText: ⓒ 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixsyncoutboundstatequeue.h"

#include "jsonio.h"
#include "logging.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaEnum>
#include <QStandardPaths>

using namespace Qt::Literals;

QJsonObject MatrixSyncOutboundStateQueue::StateChange::toJson(const MatrixSyncOutboundStateQueue::StateChange &change)
{
    return QJsonObject{{
        {"type"_L1, QLatin1StringView(QMetaEnum::fromType<MatrixSyncOutboundStateQueue::ChangeType>().valueToKey(change.type))},
        {"id"_L1, change.id},
        {"context"_L1, change.context},
    }};
}

std::optional<MatrixSyncOutboundStateQueue::StateChange> MatrixSyncOutboundStateQueue::StateChange::fromJson(const QJsonObject &obj)
{
    bool ok = false;
    const auto type = QMetaEnum::fromType<MatrixSyncOutboundStateQueue::ChangeType>().keyToValue(obj.value("type"_L1).toString().toUtf8().constData(), &ok);
    if (!ok) {
        return {};
    }
    return MatrixSyncOutboundStateQueue::StateChange{
        .type = static_cast<MatrixSyncOutboundStateQueue::ChangeType>(type),
        .id = obj.value("id"_L1).toString(),
        .context = obj.value("context"_L1).toString(),
    };
}

bool MatrixSyncOutboundStateQueue::StateChange::operator==(const MatrixSyncOutboundStateQueue::StateChange &other) const
{
    return type == other.type && id == other.id && context == other.context;
}

MatrixSyncOutboundStateQueue::MatrixSyncOutboundStateQueue(QObject *parent) :
    QObject(parent)
{
    load();
}

MatrixSyncOutboundStateQueue::~MatrixSyncOutboundStateQueue() = default;

void MatrixSyncOutboundStateQueue::append(ChangeType type, const QString &id, const QString &context)
{
    if (m_suspended) {
        return;
    }

    StateChange change{ .type = type, .id = id, .context = context};
    if (m_pendingChanges.size() > 1 && std::ranges::find(m_pendingChanges, change) != m_pendingChanges.end()) {
        return;
    }

    m_pendingChanges.push_back(std::move(change));
    qCDebug(Log) << "queuing local state change" << type << id << m_pendingChanges.size();
    store();
    Q_EMIT queueChanged();
    if (m_pendingChanges.size() == 1) {
        doReplayNext();
    }
}

void MatrixSyncOutboundStateQueue::replayNext()
{
    assert(!m_pendingChanges.empty());
    qCDebug(Log) << "completed state change" << m_pendingChanges.front().type << m_pendingChanges.front().id << m_pendingChanges.size();

    m_pendingChanges.pop_front();
    store();
    Q_EMIT queueChanged();
    if (!m_pendingChanges.empty()) {
        QMetaObject::invokeMethod(this, &MatrixSyncOutboundStateQueue::doReplayNext, Qt::QueuedConnection);
    }
}

void MatrixSyncOutboundStateQueue::retry()
{
    doReplayNext();
}

void MatrixSyncOutboundStateQueue::suspend()
{
    assert(!m_suspended);
    m_suspended = true;
}

bool MatrixSyncOutboundStateQueue::isSuspended() const
{
    return m_suspended;
}

void MatrixSyncOutboundStateQueue::resume()
{
    assert(m_suspended);
    m_suspended = false;
}

QString MatrixSyncOutboundStateQueue::path()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/matrixsync/localstatequeue"_L1;
}

void MatrixSyncOutboundStateQueue::load()
{
    QFile f(path());
    if (!f.open(QFile::ReadOnly)) {
        return; // no pending changes
    }

    const auto array = JsonIO::read(f.readAll()).toArray();
    for (const auto &obj : array) {
        auto c = StateChange::fromJson(obj.toObject());
        if (c) {
            m_pendingChanges.push_back(std::move(*c));
        }
    }
}

void MatrixSyncOutboundStateQueue::store()
{
    if (m_pendingChanges.empty()) {
        QFile::remove(MatrixSyncOutboundStateQueue::path());
    } else {
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/matrixsync/"_L1);

        QJsonArray array;
        for (const auto &change :m_pendingChanges) {
            array.push_back(StateChange::toJson(change));
        }
        QFile f(path());
        if (!f.open(QFile::WriteOnly | QFile::Truncate)) {
            qCWarning(Log) << f.errorString() << f.fileName();
            return;
        }
        f.write(JsonIO::write(array));
    }
}

void MatrixSyncOutboundStateQueue::doReplayNext()
{
    if (m_pendingChanges.empty()) {
        return;
    }

    const auto &c = m_pendingChanges.front();
    qCDebug(Log) << "replaying local state" << c.type << c.id;
    switch (c.type) {
        case BatchChange:
            Q_EMIT batchChanged(c.id, c.context);
            break;
        case BatchRemove:
            Q_EMIT batchRemoved(c.id, c.context);
            break;
        case LiveDataChange:
            Q_EMIT liveDataChanged(c.id);
            break;
        case TransferChange:
        {
            const auto [id, align] = Transfer::parseIdentifier(c.id);
            Q_EMIT transferChanged(id, align);
            break;
        }
        case DocumentAdd:
            Q_EMIT documentAdded(c.id, c.context);
            break;
        case PkPassChange:
            Q_EMIT pkPassChanged(c.id, c.context);
            break;
        case TripGroupAdded:
            Q_EMIT tripGroupAdded(c.id);
            break;
        case TripGroupChanged:
            Q_EMIT tripGroupChanged(c.id);
            break;
    }
}

#include "moc_matrixsyncoutboundstatequeue.cpp"
