// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once


#include <QAbstractListModel>
#include <KConfigGroup>
#include <KSharedConfig>

#include "accounttype.h"
#include "accounttypemodel.h"

class AccountTypeModel;
class AbstractAccount;
class QNetworkAccessManager;

class AccountModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(AccountTypeModel *accountTypeModel READ accountTypeModel CONSTANT)

public:
    enum ExtraRoles: std::uint16_t {
        IdentifierRole = Qt::UserRole + 1,
        NameRole,
        AccountNameRole,
        IconNameRole,
        AccountRole,
    };

    explicit AccountModel(QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE AbstractAccount *create(const QString &identifier);
    Q_INVOKABLE void remove(AbstractAccount *account);

    bool handleCallback(const QUrl &url);

    void setNetworkAccessManagerFactory(const std::function<QNetworkAccessManager *()> &namFactory);

    [[nodiscard]] AccountTypeModel *accountTypeModel() const;

private:
    AbstractAccount *accountFromIdentifier(const QString &identifier);
    [[nodiscard]] AccountType accountTypeFromIdentifier(const QString &identifier) const;
    void initAccount(AbstractAccount *account);

    std::vector<AbstractAccount *> m_accounts;
    std::function<QNetworkAccessManager *()> m_namFactory;
    KSharedConfigPtr m_config;
    std::vector<AccountType> m_accountTypes;
    AccountTypeModel *m_accountTypeModel = nullptr;
};
