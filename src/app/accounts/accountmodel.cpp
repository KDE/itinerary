// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "accountmodel.h"

#include "accounts/abstractaccount.h"
#include "accounts/accounttypemodel.h"
#include "accounts/deutschbahn/deutschbahnaccount.h"

#include <QUrl>
#include <QTimer>
#include <KLocalizedString>

using namespace Qt::StringLiterals;

AccountModel::AccountModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_config(KSharedConfig::openStateConfig())
    , m_accountTypes{
        {
            .identifier = u"deutschbahn"_s,
            .name = i18nc("@label", "Deutsch Bahn"),
            .iconName = u"accounts/deutschbahn/account-deutschbahn.svg"_s,
            .description = i18nc("@info", "Sync your tickets from your Deutsch Bahn account"),
            .protocolOAuthCallback = u"dbnav"_s,
        },
    }
    , m_accountTypeModel(new AccountTypeModel(m_accountTypes, this))
{
}

int AccountModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_accounts.size());
}

QVariant AccountModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));

    const auto &account = m_accounts[index.row()];
    const auto accountType = accountTypeFromIdentifier(account->identifier());

    switch (role) {
    case IdentifierRole:
        return accountType.identifier;
    case AccountNameRole:
        return account->accountName();
    case NameRole:
    case Qt::DisplayRole:
        return accountType.name;
    case IconNameRole:
        return accountType.iconName;
    case AccountRole:
        return QVariant::fromValue(account);
    default:
        return {};
    }
}

QHash<int, QByteArray> AccountModel::roleNames() const
{
    return {
        { IdentifierRole, "identifier" },
        { NameRole, "name" },
        { IconNameRole, "iconName" },
        { AccountNameRole, "accountName" },
        { AccountRole, "account" },
    };
}

bool AccountModel::handleCallback(const QUrl &url)
{
    for (const auto &account : std::as_const(m_accounts)) {
        const auto accountType = accountTypeFromIdentifier(account->identifier());
        if (accountType.protocolOAuthCallback == url.scheme()) {
            if (account->handleCallback(url)) {
                return true;
            }
        }
    }
    return false;
}

void AccountModel::setNetworkAccessManagerFactory(const std::function<QNetworkAccessManager *()> &namFactory)
{
    m_namFactory = namFactory;

    beginResetModel();
    const auto keyList = m_config->groupList();
    for (const auto &key : keyList) {
        if (!key.startsWith("account-"_L1)) {
            continue;
        }

        const auto group = m_config->group(key);
        auto identifier = group.readEntry("identifier");
        auto account = accountFromIdentifier(identifier);
        account->init(identifier, namFactory);
        initAccount(account);
        account->loadAccount(group);
        m_accounts.push_back(account);
    }
    endResetModel();
}

AbstractAccount *AccountModel::create(const QString &identifier)
{
    auto account = accountFromIdentifier(identifier);

    account->init(identifier, m_namFactory);

    beginInsertRows({}, (int)m_accounts.size(), (int)m_accounts.size());
    m_accounts.push_back(account);
    endInsertRows();

    initAccount(account);

    QTimer::singleShot(0, account, [account]() {
        account->startOAuth();
    });
    return account;
}

void AccountModel::remove(AbstractAccount *account)
{
    const auto it = std::ranges::find(std::as_const(m_accounts), account);
    if (it != m_accounts.cend()) {
        const auto index = std::distance(m_accounts.cbegin(), it);
        beginRemoveRows({}, index, index);
        m_config->deleteGroup(u"account-"_s + account->identifier() + u'/' + account->accountId());
        m_config->sync();
        m_accounts.erase(it);
        endRemoveRows();
    }
}

void AccountModel::initAccount(AbstractAccount *account)
{
    connect(account, &AbstractAccount::changed, this, [this, account](const QString &accountId) {
        auto group = m_config->group(u"account-"_s + account->identifier() + u'/' + accountId);
        group.writeEntry("identifier", account->identifier());
        account->saveAccount(group);
        m_config->sync();
    });

    connect(account, &AbstractAccount::accountNameChanged, this, [this, account]{
        const auto idx = (int)std::distance(m_accounts.begin(), std::ranges::find(m_accounts, account));
        Q_EMIT dataChanged(index(idx, 0), index(idx, 0), {AccountNameRole});
    });
}

AbstractAccount *AccountModel::accountFromIdentifier(const QString &identifier)
{
    AbstractAccount *account = nullptr;
    if (identifier == "deutschbahn"_L1) {
        account = new DeutschBahnAccount(this);
    }
    Q_ASSERT(account);
    return account;
}

AccountType AccountModel::accountTypeFromIdentifier(const QString &identifier) const
{
    for (const auto &accountType : m_accountTypes) {
        if (accountType.identifier == identifier) {
            return accountType;
        }
    }

    Q_ASSERT(false);
    return {};
}

AccountTypeModel *AccountModel::accountTypeModel() const
{
    return m_accountTypeModel;
}
