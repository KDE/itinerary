// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "matrixmanager.h"

#include <Quotient/accountregistry.h>
#include <Quotient/room.h>
#include <Quotient/qt_connection_util.h>
#include "Quotient/settings.h"

#include <KLocalizedString>
#include <QCoreApplication>

using namespace Quotient;

MatrixManager::MatrixManager(QObject *parent)
    : QObject(parent)
{
    Accounts.invokeLogin();
    connect(&Accounts, &AccountRegistry::rowsInserted, this, [this](){
        Q_EMIT connectedChanged();
        Q_EMIT userIdChanged();
        Q_EMIT connectionChanged();
        setInfoString(i18n("Syncing…"));
        connectSingleShot(Accounts.accounts()[0], &Connection::syncDone, this, [this](){
            setInfoString({});
            Accounts.accounts()[0]->stopSync();
        });
    });
    connect(&Accounts, &AccountRegistry::rowsRemoved, this, [this](){
        Q_EMIT connectedChanged();
        Q_EMIT userIdChanged();
        Q_EMIT connectionChanged();
    });
}

void MatrixManager::login(const QString &matrixId, const QString &password)
{
    auto connection = new Connection(this);
    connection->resolveServer(matrixId);
    connectSingleShot(connection, &Connection::loginFlowsChanged, this, [this, connection, matrixId, password](){
        if (!connection->supportsPasswordAuth()) {
            setInfoString(i18n("This server does not support logging in using a password"));
        }
        auto username = matrixId.mid(1, matrixId.indexOf(QLatin1Char(':')) - 1);
        connection->loginWithPassword(username, password, qAppName(), {});
    });

    connect(connection, &Connection::connected, this, [this, connection] {
        AccountSettings account(connection->userId());
        account.setKeepLoggedIn(true);
        account.setHomeserver(connection->homeserver());
        account.setDeviceId(connection->deviceId());
        account.setDeviceName(qAppName());
        account.sync();
        Q_EMIT connectedChanged();

        setInfoString(i18n("Syncing…"));
        connectSingleShot(connection, &Connection::syncDone, this, [connection, this](){
            connection->stopSync();
            setInfoString({});
        });
    });

    connect(connection, &Connection::loginError, this, [this](const QString &message) {
        setInfoString(message);
    });

    setInfoString(i18n("Logging in"));
}

QString MatrixManager::infoString() const
{
    return m_infoString;
}

bool MatrixManager::connected() const
{
    return Accounts.count() > 0;
}

void MatrixManager::setInfoString(const QString &infoString)
{
    m_infoString = infoString;
    Q_EMIT infoStringChanged();
}

QString MatrixManager::userId() const
{
    return Accounts.count() > 0 ? Accounts.accounts()[0]->userId() : QString();
}

void MatrixManager::logout()
{
    Accounts.accounts()[0]->logout();
}

Quotient::Connection *MatrixManager::connection() const
{
    if (Accounts.count() > 0) {
        return Accounts.accounts()[0];
    }
    return nullptr;
}

void MatrixManager::sync()
{
    auto connection = Accounts.accounts()[0];
    connection->sync();
    setInfoString(i18n("Syncing…"));
    connectSingleShot(connection, &Connection::syncDone, this, [this](){
        setInfoString({});
    });
}

void MatrixManager::postEvent(const QString &roomId, const QString &type, const QJsonObject &content)
{
    Accounts.accounts()[0]->room(roomId)->postJson(type, content);
}

void MatrixManager::postLocation(const QString &roomId, float latitude, float longitude, const QString &description)
{
    QJsonObject content{
            {QLatin1String("body"), description},
            {QLatin1String("msgtype"), QLatin1String("m.location")},
            {QLatin1String("org.matrix.msc3488.asset"), QJsonObject {
                    {QLatin1String("type"), QLatin1String("m.pin")}, // TODO location type (hotel, restaurant, train station, ...) here?
                    //{"name", description} // TODO location description ("Volker's Hotel") here?
            }},
            {QLatin1String("org.matrix.msc3488.ts"), QDateTime::currentDateTime().toMSecsSinceEpoch()},
            {QLatin1String("geo_uri"), QLatin1String("geo:%1,%2").arg(QString::number(latitude), QString::number(longitude))},
            {QLatin1String("org.matrix.msc1767.text"), description},
            {QLatin1String("org.matrix.msc3488.location"), QJsonObject {
                {QLatin1String("uri"), QLatin1String("geo:%1,%2").arg(QString::number(latitude), QString::number(longitude))},
                {QLatin1String("description"), description}
            }}
    };

    postEvent(roomId, QLatin1String("m.room.message"), content);
}

#include "moc_matrixmanager.cpp"
