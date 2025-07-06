/*
    SPDX-FileCopyrightText: ⓒ 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixsynclocalstatequeue.h"

#include "jsonio.h"
#include "logging.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaEnum>
#include <QStandardPaths>

using namespace Qt::Literals;

QJsonObject MatrixSyncLocalStateQueue::StateChange::toJson(const MatrixSyncLocalStateQueue::StateChange &change)
{
    return QJsonObject{{
        {"type"_L1, QLatin1StringView(QMetaEnum::fromType<MatrixSyncLocalStateQueue::ChangeType>().valueToKey(change.type))},
        {"id"_L1, change.id},
        {"context"_L1, change.context},
    }};
}

std::optional<MatrixSyncLocalStateQueue::StateChange> MatrixSyncLocalStateQueue::StateChange::fromJson(const QJsonObject &obj)
{
    bool ok = false;
    const auto type = QMetaEnum::fromType<MatrixSyncLocalStateQueue::ChangeType>().keyToValue(obj.value("type"_L1).toString().toUtf8().constData(), &ok);
    if (!ok) {
        return {};
    }
    return MatrixSyncLocalStateQueue::StateChange{
        .type = static_cast<MatrixSyncLocalStateQueue::ChangeType>(type),
        .id = obj.value("id"_L1).toString(),
        .context = obj.value("context"_L1).toString(),
    };
}

bool MatrixSyncLocalStateQueue::StateChange::operator==(const MatrixSyncLocalStateQueue::StateChange &other) const
{
    return type == other.type && id == other.id && context == other.context;
}

MatrixSyncLocalStateQueue::MatrixSyncLocalStateQueue(QObject *parent) :
    QObject(parent)
{
    load();
}

MatrixSyncLocalStateQueue::~MatrixSyncLocalStateQueue() = default;

void MatrixSyncLocalStateQueue::append(ChangeType type, const QString &id, const QString &context)
{
    if (m_suspended) {
        return;
    }

    StateChange change{ .type = type, .id = id, .context = context};
    if (m_pendingChanges.size() > 1 && std::ranges::find(m_pendingChanges, change) != m_pendingChanges.end()) {
        return;
    }

    m_pendingChanges.push_back(std::move(change));
    qDebug() << "queuing local state change" << type << id << m_pendingChanges.size();
    store();
    if (m_pendingChanges.size() == 1) {
        doReplayNext();
    }
}

void MatrixSyncLocalStateQueue::replayNext()
{
    assert(!m_pendingChanges.empty());
    qDebug() << "completed state change" << m_pendingChanges.front().type << m_pendingChanges.front().id << m_pendingChanges.size();

    m_pendingChanges.pop_front();
    store();
    if (!m_pendingChanges.empty()) {
        QMetaObject::invokeMethod(this, &MatrixSyncLocalStateQueue::doReplayNext, Qt::QueuedConnection);
    }
}

void MatrixSyncLocalStateQueue::retry()
{
    doReplayNext();
}

void MatrixSyncLocalStateQueue::suspend()
{
    assert(!m_suspended);
    m_suspended = true;
}

bool MatrixSyncLocalStateQueue::isSuspended() const
{
    return m_suspended;
}

void MatrixSyncLocalStateQueue::resume()
{
    assert(m_suspended);
    m_suspended = false;
}

QString MatrixSyncLocalStateQueue::path()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/matrixsync/localstatequeue"_L1;
}

void MatrixSyncLocalStateQueue::load()
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

void MatrixSyncLocalStateQueue::store()
{
    if (m_pendingChanges.empty()) {
        QFile::remove(MatrixSyncLocalStateQueue::path());
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

void MatrixSyncLocalStateQueue::doReplayNext()
{
    if (m_pendingChanges.empty()) {
        return;
    }

    const auto &c = m_pendingChanges.front();
    qDebug() << "replaying local state" << c.type << c.id;
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
    }
}

#include "moc_matrixsynclocalstatequeue.cpp"
