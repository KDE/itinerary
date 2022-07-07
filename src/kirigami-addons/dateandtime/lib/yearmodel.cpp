// SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "yearmodel.h"

#include <QDate>
#include <QCalendar>
#include <QLocale>

YearModel::YearModel(QObject *parent)
    : QAbstractListModel(parent)
{
    setYear(QDate::currentDate().year());
}

YearModel::~YearModel()
{}

int YearModel::year() const
{
    return m_year;
}

void YearModel::setYear(int year)
{
    if (m_year == year) {
        return;
    }
    if (QCalendar().monthsInYear(m_year) != QCalendar().monthsInYear(year)) {
        beginResetModel();
        m_year = year;
        endResetModel();
        Q_EMIT yearChanged();
        return;
    }
    m_year = year;
    Q_EMIT yearChanged();
}

int YearModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return QCalendar().monthsInYear(m_year);
    }
    return 0;
}

QVariant YearModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return QVariant();
    }
    if (role == Qt::DisplayRole) {
        // model indexes 0-11, months are 1-12
        return QLocale().monthName(index.row()+1, QLocale::ShortFormat);
    }
    return QVariant();
}

