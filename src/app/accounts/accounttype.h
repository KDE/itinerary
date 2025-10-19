// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QString>

struct AccountType {
public:
    QString identifier;
    QString name;
    QString iconName;
    QString description;
    QString protocolOAuthCallback;
};
