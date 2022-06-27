/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALENDARCORE_CALENDARLISTMODEL_H
#define KCALENDARCORE_CALENDARLISTMODEL_H

#include "kcalendarcoreextras_export.h"

#include <QAbstractListModel>

#include <memory>

namespace KCalendarCoreExtras {

class CalendarListModelPrivate;

/** Model adaptor for KCalendarCore::CalendarPlugin::calendars(). */
class KCALENDARCOREEXTRAS_EXPORT CalendarListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit CalendarListModel(QObject *parent = nullptr);
    ~CalendarListModel();

    enum Role {
        CalendarRole = Qt::UserRole,
        AccessModeRole,
    };

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    std::unique_ptr<CalendarListModelPrivate> d;
};

}

#endif // KCALENDARCORE_CALENDARLISTMODEL_H
