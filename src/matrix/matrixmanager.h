// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef MATRIXMANAGER_H
#define MATRIXMANAGER_H

#include <QObject>

#include <Quotient/accountregistry.h>
#include <Quotient/connection.h>

class MatrixManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString infoString READ infoString NOTIFY infoStringChanged)
    Q_PROPERTY(QString userId READ userId NOTIFY userIdChanged)
    Q_PROPERTY(bool isVerifiedSession READ isVerifiedSession NOTIFY sessionVerifiedChanged)
    Q_PROPERTY(Quotient::Connection *connection READ connection NOTIFY connectionChanged)

public:
    explicit MatrixManager(QObject *parent = nullptr);

    /** Device name to be used for new connections. */
    void setDeviceName(const QString &deviceName);

    /**
     * Log in to a connection
     * @param matrixId user id in the form @user:server.tld
     * @param password
     */
    Q_INVOKABLE void login(const QString &matrixId, const QString &password);

    /**
     * Log out of the connection
     */
    Q_INVOKABLE void logout();

    /**
     * Run a single sync. We're not syncing constantly, since we typically don't need it and it consumes a lot of data
     */
    Q_INVOKABLE void sync();

    [[nodiscard]] QString infoString() const;
    [[nodiscard]] bool connected() const;
    [[nodiscard]] QString userId() const;
    [[nodiscard]] bool isVerifiedSession() const;
    [[nodiscard]] Quotient::Connection *connection() const;

    Q_INVOKABLE void postLocation(const QString &roomId, float latitude, float longitude, const QString &description);
    void postEvent(const QString &roomId, const QString &type, const QJsonObject &content);

Q_SIGNALS:
    void connectedChanged();
    void infoStringChanged();
    void userIdChanged();
    void sessionVerifiedChanged();
    void connectionChanged();

private:
    QString m_infoString;
    QString m_deviceName;
    Quotient::AccountRegistry m_accountRegistry;

    void setInfoString(const QString &infoString);
};

#endif // MATRIXMANAGER_H
