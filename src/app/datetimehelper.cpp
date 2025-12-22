/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "datetimehelper.h"

#include <QDateTime>
#include <QTimeZone>

bool DateTimeHelper::isSameDateTime(const QDateTime &lhs, const QDateTime &rhs)
{
    if (lhs == rhs) {
        return true;
    }
    if (lhs.timeSpec() == Qt::LocalTime && rhs.timeSpec() != Qt::LocalTime) {
        QDateTime dt(rhs);
        dt.setTimeZone(QTimeZone::LocalTime);
        return lhs == dt;
    }
    if (lhs.timeSpec() != Qt::LocalTime && rhs.timeSpec() == Qt::LocalTime) {
        QDateTime dt(lhs);
        dt.setTimeZone(QTimeZone::LocalTime);
        return dt == rhs;
    }
    return false;
}
