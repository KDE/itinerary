// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <KConfigGroup>
#include <QAction>

class QNetworkAccessManager;

class AbstractAccount : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QAction *> actions READ actions CONSTANT)

public:
    explicit AbstractAccount(QObject *parent = nullptr);

    void init(const QString &identifier, const std::function<QNetworkAccessManager *()> &namFactory);

    [[nodiscard]] virtual QString accountName() const = 0;
    [[nodiscard]] QString identifier() const;

    virtual void loadAccount(const KConfigGroup &configGroup) = 0;
    virtual void saveAccount(KConfigGroup &configGroup) const = 0;

    virtual bool handleCallback(const QUrl &url);

    virtual void startOAuth() = 0;

    [[nodiscard]] virtual QList<QAction *> actions() const = 0;

Q_SIGNALS:
    void changed(const QString &accountId);
    void accountNameChanged();

protected:
    std::function<QNetworkAccessManager *()> m_namFactory;
    QString m_identifier;
};