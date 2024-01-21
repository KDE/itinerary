// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QNetworkReply>

class TraewellingController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString username READ username NOTIFY usernameChanged)
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY isLoggedInChanged)
public:
    enum UploadStatus {
        Success,
        Error,
    };
    Q_ENUM(UploadStatus);
    explicit TraewellingController(std::function<QNetworkAccessManager*()> namFactory, QObject *parent = nullptr);
    Q_INVOKABLE void login();
    Q_INVOKABLE void logout(bool server = true);
    [[nodiscard]] QString username() const;
    [[nodiscard]] bool isLoggedIn() const;

    Q_INVOKABLE void checkin(const QString &departureStationName, const QString &destinationStationName, const QDateTime &departureTime, const QDateTime &arrivalTime, const QString &directionStationName);

Q_SIGNALS:
    void usernameChanged();
    void isLoggedInChanged();
    void uploadStatus(UploadStatus status);
private:
    QString m_username;
    QString m_accessToken;
    void loadData();
    void get(const QString &endpoint, const QUrlQuery &query, const std::function<void(QJsonObject)> &then);
    void post(const QString &endpoint, const QByteArray &data, const std::function<void(QJsonObject, QNetworkReply::NetworkError)> &thenf);
    void setUsername(const QString &username);
    void setAccessToken(const QString &accessToken);
    QByteArray m_verifier;
    QString m_state;
    std::function<QNetworkAccessManager*()> m_namFactory;
};
