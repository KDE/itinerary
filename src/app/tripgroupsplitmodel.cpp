/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tripgroupsplitmodel.h"

#include "reservationhelper.h"
#include "reservationmanager.h"

#include <KItinerary/SortUtil>

#include <QDateTime>
#include <QLocale>

using namespace Qt::Literals;
using namespace KItinerary;

TripGroupSplitModel::TripGroupSplitModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

TripGroupSplitModel::~TripGroupSplitModel() = default;

void TripGroupSplitModel::setElements(const QStringList &elements)
{
    beginResetModel();
    m_elements = elements;
    m_beginSelection = 0;
    m_endSelection = m_elements.size() / 2;
    endResetModel();
}

QStringList TripGroupSplitModel::selection() const
{
    QStringList sel;
    if (m_elements.empty()) {
        return sel;
    }

    sel.reserve(m_endSelection - m_beginSelection);
    std::copy(m_elements.begin() + m_beginSelection, m_elements.begin() + m_endSelection, std::back_inserter(sel));
    return sel;
}

int TripGroupSplitModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return (int)m_elements.size();
}

QVariant TripGroupSplitModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid)) {
        return {};
    }

    const auto res = m_resMgr->reservation(m_elements.at(index.row()));
    switch (role) {
    case Qt::DisplayRole:
        return ReservationHelper::label(res);
    case SubtitleRole:
        if (SortUtil::hasStartTime(res)) {
            return QLocale().toString(SortUtil::startDateTime(res));
        }
        return QLocale().toString(SortUtil::startDateTime(res).date());
    case IconNameRole:
        return ReservationHelper::defaultIconName(res);
    case SelectedRole:
        return index.row() >= m_beginSelection && index.row() < m_endSelection;
    }

    return {};
}

bool TripGroupSplitModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid)) {
        return false;
    }

    if (role == SelectedRole) {
        if (value.toBool()) {
            if (index.row() == rowCount() - 1) {
                m_beginSelection = m_endSelection;
                m_endSelection = rowCount();
            } else if (index.row() == 0) {
                m_endSelection = m_beginSelection;
                m_beginSelection = 0;
            } else if (m_beginSelection == 0) {
                m_endSelection = index.row() + 1;
            } else {
                m_beginSelection = index.row();
            }
        } else {
            if (index.row() == rowCount() - 1) {
                m_endSelection = m_beginSelection;
                m_beginSelection = 0;
            } else if (index.row() == 0) {
                m_beginSelection = m_endSelection;
                m_endSelection = rowCount();
            } else if (m_beginSelection == 0) {
                m_endSelection = index.row();
            } else {
                m_beginSelection = index.row() + 1;
            }
        }

        assert(m_beginSelection < m_endSelection);
        assert(m_beginSelection < rowCount());
        assert(m_endSelection > 0);
        assert(m_endSelection <= rowCount());
        assert(m_endSelection - m_beginSelection < rowCount());

        Q_EMIT dataChanged(this->index(0, 0), this->index(rowCount() - 1, 0));
        Q_EMIT selectionChanged();
        return true;
    }

    return false;
}

QHash<int, QByteArray> TripGroupSplitModel::roleNames() const
{
    auto n = QAbstractListModel::roleNames();
    n.insert(TitleRole, "title");
    n.insert(SubtitleRole, "subtitle");
    n.insert(IconNameRole, "iconName");
    n.insert(SelectedRole, "selected");
    return n;
}

#include "moc_tripgroupsplitmodel.cpp"
