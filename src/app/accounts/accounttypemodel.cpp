// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "accounttypemodel.h"

#include <KLocalizedString>

using namespace Qt::StringLiterals;

AccountTypeModel::AccountTypeModel(const std::vector<AccountType> &accountTypes, QObject *parent)
    : QAbstractListModel(parent)
    , m_accountTypes(accountTypes)
{
    Q_ASSERT(m_accountTypes.size() > 0);
}

int AccountTypeModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_accountTypes.size());
}

QVariant AccountTypeModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));

    const auto &accountType = m_accountTypes[index.row()];

    switch (role) {
    case IdentifierRole:
        return accountType.identifier;
    case NameRole:
    case Qt::DisplayRole:
        return accountType.name;
    case IconNameRole:
        return accountType.iconName;
    case DescriptionRole:
        return accountType.description;
    case ProtocolRole:
        return accountType.protocolOAuthCallback;
    default:
        return {};
    }
}

QHash<int, QByteArray> AccountTypeModel::roleNames() const
{
    return {
        { IdentifierRole, "identifier" },
        { NameRole, "name" },
        { IconNameRole, "iconName" },
        { DescriptionRole, "description" },
        { ProtocolRole, "protocol" },
    };
}
