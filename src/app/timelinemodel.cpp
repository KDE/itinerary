/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "timelinemodel.h"
#include "pkpassmanager.h"

#include <KPkPass/Pass>

#include <QDateTime>
#include <QDebug>
#include <QLocale>

TimelineModel::TimelineModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

TimelineModel::~TimelineModel() = default;

void TimelineModel::setPkPassManager(PkPassManager* mgr)
{
    beginResetModel();
    m_mgr = mgr;
    m_passes = mgr->passes();
    std::sort(m_passes.begin(), m_passes.end(), [this](const QString &lhs, const QString &rhs) {
        return PkPassManager::relevantDate(m_mgr->pass(lhs)) > PkPassManager::relevantDate(m_mgr->pass(rhs));
    });
    connect(mgr, &PkPassManager::passAdded, this, &TimelineModel::passAdded);
    connect(mgr, &PkPassManager::passUpdated, this, &TimelineModel::passUpdated);
    connect(mgr, &PkPassManager::passRemoved, this, &TimelineModel::passRemoved);
    endResetModel();
}

int TimelineModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_mgr)
        return 0;
    return m_passes.size();
}

QVariant TimelineModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_mgr)
        return {};

    switch (role) {
        case PassRole:
            return QVariant::fromValue(m_mgr->pass(m_passes.at(index.row())));
        case PassIdRole:
            return m_passes.at(index.row());
        case SectionHeader:
            return QLocale().toString(PkPassManager::relevantDate(m_mgr->pass(m_passes.at(index.row()))).date(), QLocale::ShortFormat);
    }
    return {};
}

QHash<int, QByteArray> TimelineModel::roleNames() const
{
    auto names = QAbstractListModel::roleNames();
    names.insert(PassRole, "pass");
    names.insert(PassIdRole, "passId");
    names.insert(SectionHeader, "sectionHeader");
    return names;
}

void TimelineModel::passAdded(const QString &passId)
{
    auto it = std::lower_bound(m_passes.begin(), m_passes.end(), passId, [this](const QString &lhs, const QString &rhs) {
        return PkPassManager::relevantDate(m_mgr->pass(lhs)) > PkPassManager::relevantDate(m_mgr->pass(rhs));
    });
    auto index = std::distance(m_passes.begin(), it);
    beginInsertRows({}, index, index);
    m_passes.insert(it, passId);
    endInsertRows();
}

void TimelineModel::passUpdated(const QString &passId)
{
    auto it = std::lower_bound(m_passes.begin(), m_passes.end(), passId, [this](const QString &lhs, const QString &rhs) {
        return PkPassManager::relevantDate(m_mgr->pass(lhs)) > PkPassManager::relevantDate(m_mgr->pass(rhs));
    });
    auto row = std::distance(m_passes.begin(), it);
    emit dataChanged(index(row, 0), index(row, 0));
}

void TimelineModel::passRemoved(const QString &passId)
{
    auto it = std::lower_bound(m_passes.begin(), m_passes.end(), passId, [this](const QString &lhs, const QString &rhs) {
        return PkPassManager::relevantDate(m_mgr->pass(lhs)) > PkPassManager::relevantDate(m_mgr->pass(rhs));
    });
    auto index = std::distance(m_passes.begin(), it);
    beginRemoveRows({}, index, index);
    m_passes.erase(it);
    endRemoveRows();
}
