// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "matrixmanager.h"

#include <Quotient/room.h>
#include "Quotient/settings.h"

#include <KLocalizedString>
#include <QCoreApplication>

using namespace Quotient;

MatrixManager::MatrixManager(QObject *parent)
    : QObject(parent)
{
    m_accountRegistry.invokeLogin();
    connect(&m_accountRegistry, &AccountRegistry::rowsInserted, this, [this](){
        Q_EMIT connectedChanged();
        Q_EMIT userIdChanged();
        Q_EMIT connectionChanged();
        setInfoString(i18n("Syncing…"));
        connect(m_accountRegistry.accounts()[0], &Connection::syncDone, this, [this](){
            setInfoString({});
            m_accountRegistry.accounts()[0]->stopSync();
        }, Qt::SingleShotConnection);
    });
    connect(&m_accountRegistry, &AccountRegistry::rowsRemoved, this, [this](){
        Q_EMIT connectedChanged();
        Q_EMIT userIdChanged();
        Q_EMIT connectionChanged();
    });
}

void MatrixManager::login(const QString &matrixId, const QString &password)
{
    auto connection = new Connection(this);
    connection->resolveServer(matrixId);
    connect(connection, &Connection::loginFlowsChanged, this, [this, connection, matrixId, password](){
        if (!connection->supportsPasswordAuth()) {
            setInfoString(i18n("This server does not support logging in using a password"));
        }
        auto username = matrixId.mid(1, matrixId.indexOf(QLatin1Char(':')) - 1);
        connection->loginWithPassword(username, password, qAppName(), {});
    }, Qt::SingleShotConnection);

    connect(connection, &Connection::connected, this, [this, connection] {
        AccountSettings account(connection->userId());
        account.setKeepLoggedIn(true);
        account.setHomeserver(connection->homeserver());
        account.setDeviceId(connection->deviceId());
        account.setDeviceName(qAppName());
        account.sync();
        Q_EMIT connectedChanged();

        setInfoString(i18n("Syncing…"));
        connect(connection, &Connection::syncDone, this, [connection, this](){
            connection->stopSync();
            setInfoString({});
        }, Qt::SingleShotConnection);
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
    return m_accountRegistry.count() > 0;
}

void MatrixManager::setInfoString(const QString &infoString)
{
    m_infoString = infoString;
    Q_EMIT infoStringChanged();
}

QString MatrixManager::userId() const
{
    return m_accountRegistry.count() > 0 ? m_accountRegistry.accounts()[0]->userId() : QString();
}

void MatrixManager::logout()
{
    m_accountRegistry.accounts()[0]->logout();
}

Quotient::Connection *MatrixManager::connection() const
{
    if (m_accountRegistry.count() > 0) {
        return m_accountRegistry.accounts()[0];
    }
    return nullptr;
}

void MatrixManager::sync()
{
    auto connection = m_accountRegistry.accounts()[0];
    connection->sync();
    setInfoString(i18n("Syncing…"));
    connect(connection, &Connection::syncDone, this, [this](){
        setInfoString({});
    }, Qt::SingleShotConnection);
}

void MatrixManager::postEvent(const QString &roomId, const QString &type, const QJsonObject &content)
{
    m_accountRegistry.accounts()[0]->room(roomId)->postJson(type, content);
}

void MatrixManager::postLocation(const QString &roomId, float latitude, float longitude, const QString &description)
{
    QJsonObject content{
            {QLatin1StringView("body"), description},
            {QLatin1StringView("msgtype"), QLatin1StringView("m.location")},
            {QLatin1StringView("org.matrix.msc3488.asset"), QJsonObject {
                    {QLatin1StringView("type"), QLatin1StringView("m.pin")}, // TODO location type (hotel, restaurant, train station, ...) here?
                    //{"name", description} // TODO location description ("Volker's Hotel") here?
            }},
            {QLatin1StringView("org.matrix.msc3488.ts"), QDateTime::currentDateTime().toMSecsSinceEpoch()},
            {QLatin1StringView("geo_uri"), QLatin1StringView("geo:%1,%2").arg(QString::number(latitude), QString::number(longitude))},
            {QLatin1StringView("org.matrix.msc1767.text"), description},
            {QLatin1StringView("org.matrix.msc3488.location"), QJsonObject {
                {QLatin1StringView("uri"), QLatin1StringView("geo:%1,%2").arg(QString::number(latitude), QString::number(longitude))},
                {QLatin1StringView("description"), description}
            }}
    };

    postEvent(roomId, QLatin1StringView("m.room.message"), content);
}

#include "moc_matrixmanager.cpp"
