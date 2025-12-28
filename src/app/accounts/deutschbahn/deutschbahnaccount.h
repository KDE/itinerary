// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "../abstractaccount.h"

#include <KConfigGroup>
#include <KPluginFactory>

#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthUriSchemeReplyHandler>
#include <QNetworkRequestFactory>

class DeutschBahnAccount : public AbstractAccount
{
    Q_OBJECT

public:
    explicit DeutschBahnAccount(QObject *parent);

    void loadAccount(const KConfigGroup &configGroup) override;
    void saveAccount(KConfigGroup &configGroup) const override;

    [[nodiscard]]
    QString accountName() const override;

    [[nodiscard]]
    QString accountId() const override;

    bool handleCallback(const QUrl &url) override;

    void startOAuth() override;

    [[nodiscard]] QList<QAction *> actions() const override;

private:
    void writeKeychain(const QString &key, const QJsonObject &value);
    void readAccounData();
    void setupOauth();
    void downloadTickets();

    QNetworkRequestFactory m_api;
    QOAuth2AuthorizationCodeFlow *m_oauth = nullptr;
    QOAuthUriSchemeReplyHandler m_handler;
    QString m_accountName;
    QString m_accountId;
    QAction *m_syncAction;
};
