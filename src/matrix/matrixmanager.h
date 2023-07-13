// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef MATRIXMANAGER_H
#define MATRIXMANAGER_H

#include <QObject>

#include <Quotient/connection.h>

class MatrixManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString infoString READ infoString NOTIFY infoStringChanged)
    Q_PROPERTY(QString userId READ userId NOTIFY userIdChanged)
    Q_PROPERTY(Quotient::Connection *connection READ connection NOTIFY connectionChanged)
    Q_PROPERTY(QString syncRoom READ syncRoom WRITE setSyncRoom NOTIFY syncRoomChanged)

public:
    explicit MatrixManager(QObject *parent = nullptr);

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

    QString infoString() const;
    bool connected() const;
    QString userId() const;
    Quotient::Connection *connection() const;

    Q_INVOKABLE void postLocation(const QString &roomId, float latitude, float longitude, const QString &description);
    void postEvent(const QString &roomId, const QString &type, const QJsonObject &content);

    QString syncRoom() const;
    void setSyncRoom(const QString &roomId);

Q_SIGNALS:
    void connectedChanged();
    void infoStringChanged();
    void userIdChanged();
    void connectionChanged();
    void syncRoomChanged();

private:
    QString m_infoString;

    void setInfoString(const QString &infoString);
};

#endif //MATRIXMANAGER_H
