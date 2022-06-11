/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendarlistmodel.h"
#include "calendarpluginloader.h"

#include <KCalendarCore/CalendarPlugin>

using namespace KCalendarCore;

namespace KCalendarCore {
class CalendarListModelPrivate
{
public:
    QVector<Calendar::Ptr> calendars;
};
}

CalendarListModel::CalendarListModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new CalendarListModelPrivate)
{
    if (CalendarPluginLoader::hasPlugin()) {
        d->calendars = CalendarPluginLoader::plugin()->calendars();
        connect(CalendarPluginLoader::plugin(), &CalendarPlugin::calendarsChanged, this, [this]() {
            beginResetModel();
            d->calendars = CalendarPluginLoader::plugin()->calendars();
            endResetModel();
        });
    }
}

CalendarListModel::~CalendarListModel() = default;

int CalendarListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !CalendarPluginLoader::hasPlugin()) {
        return 0;
    }

    return d->calendars.size();
}

QVariant CalendarListModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index) || !CalendarPluginLoader::hasPlugin()) {
        return {};
    }

    const auto &cal = d->calendars.at(index.row());
    switch (role) {
        case Qt::DisplayRole:
            return cal->name();
        case Qt::DecorationRole:
            return cal->icon();
        case CalendarRole:
            return QVariant::fromValue(cal.get());
        case AccessModeRole:
            return cal->accessMode();
    }

    return {};
}

QHash<int, QByteArray> CalendarListModel::roleNames() const
{
    auto n = QAbstractListModel::roleNames();
    n.insert(Qt::DisplayRole, "name");
    n.insert(CalendarRole, "calendar");
    n.insert(AccessModeRole, "accessMode");
    return n;
}
