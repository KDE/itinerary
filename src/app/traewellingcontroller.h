// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>

class TraewellingController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString username READ username NOTIFY usernameChanged)
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY isLoggedInChanged)
public:
    explicit TraewellingController(QObject *parent = nullptr);
    Q_INVOKABLE void login();
    Q_INVOKABLE void logout(bool server = true);
    QString username() const;
    bool isLoggedIn() const;

Q_SIGNALS:
    void usernameChanged();
    void isLoggedInChanged();
private:
    QString m_username;
    QString m_accessToken;
    void loadData();
    void get(const QString &endpoint, const std::function<void(QJsonObject)> &then);
    void post(const QString &endpoint, const QByteArray &data, const std::function<void(QJsonObject)> &thenf);
    void setUsername(const QString &username);
    void setAccessToken(const QString &accessToken);
    QByteArray m_verifier;
    QString m_state;
};
