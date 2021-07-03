/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "statisticstimerangemodel.h"
#include "reservationmanager.h"

#include <KItinerary/SortUtil>

#include <KLocalizedString>

#include <QDate>
#include <QDebug>
#include <QLocale>

using namespace KItinerary;

StatisticsTimeRangeModel::StatisticsTimeRangeModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

StatisticsTimeRangeModel::~StatisticsTimeRangeModel() = default;

ReservationManager* StatisticsTimeRangeModel::reservationManager() const
{
    return m_resMgr;
}

void StatisticsTimeRangeModel::setReservationManager(ReservationManager *resMgr)
{
    if (m_resMgr == resMgr) {
        return;
    }
    m_resMgr = resMgr;
    Q_EMIT setupChanged();

    beginResetModel();
    int y = 0;

    const auto &batches = m_resMgr->batches();
    for (const auto &batchId : batches) {
        const auto res = m_resMgr->reservation(batchId);
        const auto dt = SortUtil::startDateTime(res);
        if (dt.date().year() != y) {
            m_years.push_back(dt.date().year());
            y = dt.date().year();
        }
    }
    std::reverse(m_years.begin(), m_years.end());
    endResetModel();
}

int StatisticsTimeRangeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return 1 + m_years.size();
}

QVariant StatisticsTimeRangeModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
        case Qt::DisplayRole:
            if (index.row() == 0) {
                return i18n("Total");
            }
            return QLocale().toString(QDate(m_years[index.row() - 1], 1, 1), QStringLiteral("yyyy"));
        case BeginRole:
            if (index.row() == 0 || index.row() == (int)m_years.size()) { // first range is open ended here, to skip trend computation
                return QDate();
            }
            return QDate(m_years[index.row() - 1], 1, 1);
        case EndRole:
            if (index.row() == 0) {
                return QDate();
            }
            return QDate(m_years[index.row() - 1], 12, 31);
    }

    return {};
}

QHash<int, QByteArray> StatisticsTimeRangeModel::roleNames() const
{
    auto r = QAbstractListModel::roleNames();
    r.insert(BeginRole, "begin");
    r.insert(EndRole, "end");
    return r;
}
