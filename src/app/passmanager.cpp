/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "passmanager.h"
#include "genericpkpass.h"
#include "logging.h"

#include <kitinerary_version.h>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/MergeUtil>
#include <KItinerary/ProgramMembership>
#include <KItinerary/Ticket>

#include <KLocalizedString>

#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QUuid>

using namespace KItinerary;

static bool isSamePass(const GenericPkPass &lhs, const GenericPkPass &rhs)
{
    return lhs.pkpassPassTypeIdentifier() == rhs.pkpassPassTypeIdentifier() && lhs.pkpassSerialNumber() == rhs.pkpassSerialNumber();
}

QString PassManager::Entry::name() const
{
    if (JsonLd::isA<KItinerary::ProgramMembership>(data)) {
        return data.value<KItinerary::ProgramMembership>().programName();
    }
    if (JsonLd::isA<GenericPkPass>(data)) {
        return data.value<GenericPkPass>().name();
    }
    if (JsonLd::isA<KItinerary::Ticket>(data)) {
        return data.value<KItinerary::Ticket>().name();
    }
    return {};
}

QDateTime PassManager::Entry::validUntil() const
{
    if (JsonLd::isA<KItinerary::ProgramMembership>(data)) {
        return data.value<KItinerary::ProgramMembership>().validUntil();
    }
    if (JsonLd::isA<GenericPkPass>(data)) {
        return data.value<GenericPkPass>().validUnitl();
    }
    if (JsonLd::isA<KItinerary::Ticket>(data)) {
        return data.value<KItinerary::Ticket>().validUntil();
    }
    return {};
}

bool PassManager::PassComparator::operator()(const PassManager::Entry &lhs, const PassManager::Entry &rhs) const
{
    // valid before invalid, then sorted by name
    const auto lhsExpired = lhs.validUntil().isValid() && lhs.validUntil() < m_baseTime;
    const auto rhsExpired = rhs.validUntil().isValid() && rhs.validUntil() < m_baseTime;

    if (lhsExpired == rhsExpired) {
        const auto nameCmp = lhs.name().localeAwareCompare(rhs.name());
        if (nameCmp == 0) {
            return lhs.id < rhs.id;
        }
        return nameCmp < 0;
    }

    return !lhsExpired;
}

PassManager::PassManager(QObject *parent)
    : QAbstractListModel(parent)
    , m_baseTime(QDateTime::currentDateTime())
{
#if KITINERARY_VERSION >= QT_VERSION_CHECK(5, 22, 42)
    MergeUtil::registerComparator(isSamePass);
#endif

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

#if KITINERARY_VERSION < QT_VERSION_CHECK(5, 22, 42)
// ### this should eventually be replaced by custom type support in MergeUtil
static bool isSameBackwardCompat(const QVariant &lhs, const QVariant &rhs)
{
    if (!MergeUtil::isSame(lhs, rhs)) {
        return false;
    }

    if (JsonLd::isA<GenericPkPass>(lhs)) {
        const auto lhsPass = lhs.value<GenericPkPass>();
        const auto rhsPass = rhs.value<GenericPkPass>();
        return isSamePass(lhsPass, rhsPass);
    }

    return true;
}
#endif

bool PassManager::import(const QVariant &pass, const QString &id)
{
    // check if this is an element we already have
    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
#if KITINERARY_VERSION < QT_VERSION_CHECK(5, 22, 42)
        if ((!id.isEmpty() && (*it).id == id) || isSameBackwardCompat((*it).data, pass)) {
#else
        if ((!id.isEmpty() && (*it).id == id) || MergeUtil::isSame((*it).data, pass)) {
#endif
            (*it).data = MergeUtil::merge((*it).data, pass);
            write((*it).data, (*it).id);
            const auto idx = index(std::distance(m_entries.begin(), it), 0);
            Q_EMIT dataChanged(idx, idx);
            return true;
        }
    }

    if (JsonLd::isA<KItinerary::ProgramMembership>(pass) || JsonLd::isA<GenericPkPass>(pass) || JsonLd::isA<KItinerary::Ticket>(pass)) {
        Entry entry;
        entry.id = id.isEmpty() ? QUuid::createUuid().toString() : id;
        entry.data = pass;
        if (!write(entry.data, entry.id)) {
            return false;
        }

        const auto it = std::lower_bound(m_entries.begin(), m_entries.end(), entry, PassComparator(m_baseTime));
        if (it != m_entries.end() && (*it).id == entry.id) {
            (*it).data = entry.data;
            const auto idx = index(std::distance(m_entries.begin(), it), 0);
            Q_EMIT dataChanged(idx, idx);
        } else {
            const auto row = std::distance(m_entries.begin(), it);
            beginInsertRows({}, row, row);
            m_entries.insert(it, std::move(entry));
            endInsertRows();
        }
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

QString PassManager::findMatchingPass(const QVariant &pass) const
{
    if (JsonLd::isA<KItinerary::ProgramMembership>(pass)) {
        const auto program = pass.value<KItinerary::ProgramMembership>();
        if (!program.membershipNumber().isEmpty()) {
            const auto it = std::find_if(m_entries.begin(), m_entries.end(), [program](const auto &entry) {
                return entry.data.template value<KItinerary::ProgramMembership>().membershipNumber() == program.membershipNumber();
            });
            return it == m_entries.end() ? QString() : (*it).id;
        }
        if (!program.programName().isEmpty()) {
            // unique substring match
            QString id;
            for (const auto &entry : m_entries) {
                if (const auto dt = entry.validUntil(); dt.isValid() && dt < m_baseTime) {
                    continue; // expired
                }
                const auto name = entry.data.value<KItinerary::ProgramMembership>().programName();
                if (name.isEmpty() || (!name.contains(program.programName(), Qt::CaseInsensitive) && !(program.programName().contains(name, Qt::CaseInsensitive)))) {
                    continue;
                }
                if (id.isEmpty()) {
                    id = entry.id;
                } else {
                    return {};
                }
            }
            return id;
        }
    }
    return QString();
}

QVariant PassManager::pass(const QString &passId) const
{
    const auto it = std::find_if(m_entries.begin(), m_entries.end(), [passId](const auto &entry) {
        return entry.id == passId;
    });
    return it == m_entries.end() ? QVariant(): (*it).data;
}

QVariant PassManager::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    auto &entry = m_entries[index.row()];
    switch (role) {
        case PassRole:
            return entry.data;
        case PassIdRole:
            return entry.id;
        case PassTypeRole:
            if (JsonLd::isA<KItinerary::ProgramMembership>(entry.data)) {
                return ProgramMembership;
            }
            if (JsonLd::isA<GenericPkPass>(entry.data)) {
                return PkPass;
            }
            if (JsonLd::isA<KItinerary::Ticket>(entry.data)) {
                return Ticket;
            }
            return {};
        case PassDataRole:
            return rawData(entry);
        case NameRole:
            return entry.name();
        case ValidUntilRole:
            return entry.validUntil();
        case SectionRole:
            if (const auto dt = entry.validUntil(); dt.isValid() && dt < m_baseTime) {
                return i18nc("no longer valid tickets", "Expired");
            }
            return i18nc("not yet expired tickets", "Valid");
    }

    return {};
}

QHash<int, QByteArray> PassManager::roleNames() const
{
    auto r = QAbstractListModel::roleNames();
    r.insert(PassRole, "pass");
    r.insert(PassIdRole, "passId");
    r.insert(PassTypeRole, "type");
    r.insert(NameRole, "name");
    r.insert(ValidUntilRole, "validUntil");
    r.insert(SectionRole, "section");
    return r;
}

void PassManager::load()
{
    QDirIterator it(basePath(), QDir::Files);
    while (it.hasNext()) {
        it.next();
        Entry entry;
        entry.id = it.fileName();
        const auto data = rawData(entry);
        if (!data.isEmpty()) {
            entry.data = JsonLdDocument::fromJsonSingular(QJsonDocument::fromJson(data).object());
        }
        m_entries.push_back(std::move(entry));
    }
    std::sort(m_entries.begin(), m_entries.end(), PassComparator(m_baseTime));
}

bool PassManager::write(const QVariant &data, const QString &id) const
{
    auto path = basePath();
    QDir().mkpath(path);
    path += id;
    QFile f(path);
    if (!f.open(QFile::WriteOnly)) {
        qCWarning(Log) << "Failed to open file:" << f.fileName() << f.errorString();
        return false;
    }
    f.write(QJsonDocument(JsonLdDocument::toJson(data)).toJson());
    return true;
}

QByteArray PassManager::rawData(const Entry &entry) const
{
    QFile f(basePath() + entry.id);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to open file:" << f.fileName() << f.errorString();
        return {};
    }

    return f.readAll();
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
