/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "passmanager.h"

#include "genericpkpass.h"
#include "jsonio.h"
#include "logging.h"

#include <KItinerary/DocumentUtil>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/ExtractorValidator>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/MergeUtil>
#include <KItinerary/ProgramMembership>
#include <KItinerary/Ticket>

#include <KLocalizedString>

#include <QDirIterator>
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

QDateTime PassManager::Entry::validFrom() const
{
    if (JsonLd::isA<KItinerary::ProgramMembership>(data)) {
        return data.value<KItinerary::ProgramMembership>().validFrom();
    }
    if (JsonLd::isA<KItinerary::Ticket>(data)) {
        return data.value<KItinerary::Ticket>().validFrom();
    }
    return {};
}

QDateTime PassManager::Entry::validUntil() const
{
    if (JsonLd::isA<KItinerary::ProgramMembership>(data)) {
        return data.value<KItinerary::ProgramMembership>().validUntil();
    }
    if (JsonLd::isA<GenericPkPass>(data)) {
        return data.value<GenericPkPass>().validUntil();
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
    MergeUtil::registerComparator(isSamePass);
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

QString PassManager::import(const QVariant &pass, const QString &id)
{
    // check if this is an element we already have
    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
        if ((!id.isEmpty() && (*it).id == id) || MergeUtil::isSame((*it).data, pass)) {
            (*it).data = MergeUtil::merge((*it).data, pass);
            write((*it).data, (*it).id);
            const auto idx = index(std::distance(m_entries.begin(), it), 0);
            Q_EMIT dataChanged(idx, idx);
            Q_EMIT passChanged((*it).id);
            return (*it).id;
        }
    }

    if (JsonLd::isA<KItinerary::ProgramMembership>(pass) || JsonLd::isA<GenericPkPass>(pass) || JsonLd::isA<KItinerary::Ticket>(pass)) {
        Entry entry;
        entry.id = id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : id;
        entry.data = pass;
        if (!write(entry.data, entry.id)) {
            return {};
        }

        auto it = std::lower_bound(m_entries.begin(), m_entries.end(), entry, PassComparator(m_baseTime));
        if (it != m_entries.end() && (*it).id == entry.id) {
            (*it).data = entry.data;
            const auto idx = index(std::distance(m_entries.begin(), it), 0);
            Q_EMIT dataChanged(idx, idx);
            Q_EMIT passChanged((*it).id);
        } else {
            const auto row = std::distance(m_entries.begin(), it);
            beginInsertRows({}, row, row);
            it = m_entries.insert(it, std::move(entry));
            endInsertRows();
        }
        return (*it).id;
    }

    return {};
}

QStringList PassManager::import(const QVector<QVariant> &passes)
{
    ExtractorPostprocessor postproc;
    postproc.process(passes);
    const auto processed = postproc.result();

    ExtractorValidator validator;
    validator.setAcceptedTypes<KItinerary::Ticket, KItinerary::ProgramMembership>();

    QStringList result;
    for (const auto &pass : processed) {
        if (validator.isValidElement(pass)) {
            auto id = import(pass);
            if (!id.isEmpty()) {
                result.push_back(id);
            }
        }
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
            qCWarning(Log) << "Invalid pass type!" << entry.data;
            // return a valid result here, as an invalid one will completely break the delegate chooser and
            // nothing will be display at all. With a valid type we'll get an empty item that to the very least
            // allows deletion
            return Ticket;
        case PassDataRole:
            return JsonIO::convert(rawData(entry), JsonIO::JSON);
        case NameRole:
            return entry.name();
        case ValidUntilRole:
            return entry.validUntil();
        case SectionRole:
            if (const auto dt = entry.validUntil(); dt.isValid() && dt < m_baseTime) {
                return i18nc("no longer valid tickets", "Expired");
            }
            if (const auto dt = entry.validFrom(); dt.isValid() && dt > m_baseTime) {
                return i18nc("not yet valid tickets", "Future");
            }
            return i18nc("not yet expired tickets", "Valid");
        case ValidRangeLabelRole:
        {
            // TODO align this with the time labels in the trip groups? those handle a few more cases...
            const auto from = entry.validFrom();
            const auto to = entry.validUntil();
            if (!from.isValid()) {
                return {};
            }
            const auto days = from.daysTo(to);
            if (!to.isValid() || days <= 1) {
                return QLocale().toString(from.date(), QLocale::ShortFormat);
            }
            if (days >= 28 && days <= 31) {
                return QLocale().toString(from, QStringLiteral("MMMM yyyy"));
            }
            if (days >= 365 && days <= 366) {
                return QLocale().toString(from, QStringLiteral("yyyy"));
            }
            return i18n("%1 - %2", QLocale().toString(from.date(), QLocale::ShortFormat), QLocale().toString(to.date(), QLocale::ShortFormat));
        }
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
    r.insert(ValidRangeLabelRole, "validRangeLabel");
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
            entry.data = JsonLdDocument::fromJsonSingular(JsonIO::read(data).toObject());
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
    f.write(JsonIO::write(JsonLdDocument::toJson(data)));
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

void PassManager::update(const QString &passId, const QVariant &pass)
{
    auto it = std::find_if(m_entries.begin(), m_entries.end(), [&passId](const auto &lhs) { return lhs.id == passId; });
    if (it == m_entries.end()) {
        qWarning() << "couldn't find pass to update!?" << passId << pass;
        return;
    }
    (*it).data = pass;
    write((*it).data, (*it).id);
    Q_EMIT passChanged((*it).id);

    // the updated pass might have changed properties used for sorting here
    beginResetModel();
    std::sort(m_entries.begin(), m_entries.end(), PassComparator(m_baseTime));
    endResetModel();
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

QVariantList PassManager::documentIds(const QVariant& pass)
{
    return DocumentUtil::documentIds(pass);
}

QString PassManager::basePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1StringView("/programs/");
}

#include "moc_passmanager.cpp"
