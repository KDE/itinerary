// SPDX-CopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "wrappedmanager.h"

#include <QDate>

WrappedManager::WrappedManager(QObject *parent)
    : QObject(parent)
{}

bool WrappedManager::available() const
{
    const auto now = QDate::currentDate();
    return now.month() == 12 && now.day() > 15;
}
