/*
    SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MATRIXMANAGER_H
#define MATRIXMANAGER_H

#include <QObject>

class QNetworkAccessManager;


class MatrixManager : public QObject {

    Q_OBJECT

    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY isLoggedInChanged)
    Q_PROPERTY(QString userId READ userId NOTIFY userIdChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)

public:
    explicit MatrixManager(QObject *parent = nullptr);
    Q_INVOKABLE void login(const QString &matrixId, const QString &password);
    Q_INVOKABLE void logout();

    [[nodiscard]] bool isLoggedIn() const;
    [[nodiscard]] QString userId() const;
    [[nodiscard]] QString errorString() const;

Q_SIGNALS:
    void isLoggedInChanged();
    void userIdChanged();
    void errorStringChanged();

private:
    QNetworkAccessManager *m_nam;
    QString m_serverUrl;
    QString m_accessToken;
    QString m_deviceId;
    QString m_userId;
    QString m_errorString;

    // Only call this once the homeserver url has been resolved
    void doLogin(const QString &matrixId, const QString &password);

    void setUserId(const QString &userId);
    void setErrorString(const QString &errorString);
    void takeIdentity(const QString &matrixId, const QString &accessToken);
    void sync();
};


#endif //MATRIXMANAGER_H
