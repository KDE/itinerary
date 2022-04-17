/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "passmanager.h"
#include "logging.h"

#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/ProgramMembership>

#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QUuid>

using namespace KItinerary;

PassManager::PassManager(QObject *parent)
    : QAbstractListModel(parent)
{
    load();
}

PassManager::~PassManager() = default;

int PassManager::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_entries.size();
}

bool PassManager::import(const QVariant &pass)
{
    if (JsonLd::isA<KItinerary::ProgramMembership>(pass)) {
        Entry entry;
        entry.id = QUuid::createUuid().toString();
        entry.data = pass;

        auto path = basePath();
        QDir().mkpath(path);
        path += entry.id;
        QFile f(path);
        if (!f.open(QFile::WriteOnly)) {
            qCWarning(Log) << "Failed to open file:" << f.fileName() << f.errorString();
            return false;
        }
        f.write(QJsonDocument(JsonLdDocument::toJson(entry.data)).toJson());
        f.close();

        beginInsertRows({}, rowCount(), rowCount());
        m_entries.push_back(std::move(entry));
        endInsertRows();
        return true;
    }

    return false;
}

bool PassManager::import(const QVector<QVariant> &passes)
{
    ExtractorPostprocessor postproc;
    postproc.setValidationEnabled(false);
    postproc.process(passes);
    const auto processed = postproc.result();

    bool result = false;
    for (const auto &pass : processed) {
        result |= import(pass);
    }
    return result;
}

QVariant PassManager::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    auto &entry = m_entries[index.row()];
    switch (role) {
        case PassRole:
            ensureLoaded(entry);
            return entry.data;
        case PassIdRole:
            return entry.id;
        case PassTypeRole:
            ensureLoaded(entry);
            if (JsonLd::isA<KItinerary::ProgramMembership>(entry.data)) {
                return ProgramMembership;
            }
            return {};
    }

    return {};
}

QHash<int, QByteArray> PassManager::roleNames() const
{
    auto r = QAbstractListModel::roleNames();
    r.insert(PassRole, "pass");
    r.insert(PassIdRole, "passId");
    r.insert(PassTypeRole, "type");
    return r;
}

void PassManager::load()
{
    QDirIterator it(basePath(), QDir::Files);
    while (it.hasNext()) {
        it.next();
        m_entries.push_back({it.fileName(), QVariant()});
    }
}

void PassManager::ensureLoaded(Entry &entry) const
{
    if (!entry.data.isNull()) {
        return;
    }

    QFile f(basePath() + entry.id);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to open file:" << f.fileName() << f.errorString();
        return;
    }

    entry.data = JsonLdDocument::fromJsonSingular(QJsonDocument::fromJson(f.readAll()).object());
}

bool PassManager::remove(const QString &passId)
{
    auto it = std::find_if(m_entries.begin(), m_entries.end(), [passId](const auto &entry) {
        return entry.id == passId;
    });
    if (it != m_entries.end()) {
        return removeRow(std::distance(m_entries.begin(), it));
    }
    return false;
}

bool PassManager::removeRow(int row, const QModelIndex& parent)
{
    return QAbstractListModel::removeRow(row, parent);
}

bool PassManager::removeRows(int row, int count, const QModelIndex& parent)
{
    if (parent.isValid()) {
        return false;
    }

    const auto path = basePath();
    beginRemoveRows({}, row, row + count - 1);
    for (int i = row; i < row + count; ++i) {
        QFile::remove(path + m_entries[i].id);
    }
    m_entries.erase(m_entries.begin() + row, m_entries.begin() + row + count);
    endRemoveRows();
    return true;
}

QString PassManager::basePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/programs/");
}
