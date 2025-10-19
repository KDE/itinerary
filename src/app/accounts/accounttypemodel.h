// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>

#include "accounttype.h"

class AccountTypeModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ExtraRoles: std::uint16_t {
        IdentifierRole = Qt::UserRole + 1,
        NameRole,
        IconNameRole,
        DescriptionRole,
    };

    explicit AccountTypeModel(const std::vector<AccountType> &m_accountTypes, QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    std::vector<AccountType> m_accountTypes;
};
