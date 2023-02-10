/*
    SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matrixmanager.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <KLocalizedString>
#include <QCoreApplication>

#include <qtkeychain/keychain.h>
#include <QFile>

void MatrixManager::login(const QString &matrixId, const QString &password)
{
    const auto homeserver = matrixId.split(QLatin1Char(':'))[1];
    const QUrl wellKnownUrl(QStringLiteral("https://%1/.well-known/matrix/client").arg(homeserver));
    auto *const reply = m_nam->get(QNetworkRequest(wellKnownUrl));
    connect(reply, &QNetworkReply::finished, this, [homeserver, matrixId, password, this, reply](){
        if (reply->error() != QNetworkReply::NoError) {
            setErrorString(i18n("Unable to find matrix server %1", homeserver));
            return;
        }
        const auto json = QJsonDocument::fromJson(reply->readAll()).object();
        m_serverUrl = json[QStringLiteral("m.homeserver")][QStringLiteral("base_url")].toString();
        const QUrl loginUrl(QStringLiteral("%1/_matrix/client/v3/login").arg(m_serverUrl));
        auto *const reply = m_nam->get(QNetworkRequest(loginUrl));
        connect(reply, &QNetworkReply::finished, this, [matrixId, password, this, reply](){
            if (reply->error() != QNetworkReply::NoError) {
                setErrorString(i18n("Unable to determine login methods for this server: (%1) %2", reply->error(), reply->errorString()));
                return;
            }
            const auto json = QJsonDocument::fromJson(reply->readAll()).object();
            bool canLogin = false;
            for (const auto &flow : json[QStringLiteral("flows")].toArray()) {
                if (flow.toObject()[QStringLiteral("type")].toString() == QStringLiteral("m.login.password")) {
                    canLogin = true;
                    break;
                }
            }
            if (!canLogin) {
                setErrorString(i18n("Logging in to matrix servers using SSO is not yet supported"));
                return;
            }
            doLogin(matrixId, password);
        });
    });

}

MatrixManager::MatrixManager(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    auto *job = new QKeychain::ReadPasswordJob(qAppName());
    job->setKey(QStringLiteral("Matrix"));
    job->setAutoDelete(true);
    connect(job, &QKeychain::ReadPasswordJob::finished, this, [this, job](){
        if (job->error() != QKeychain::Error::NoError) {
            qWarning() << "Error reading from keychain" << job->error();
            return;
        }
        auto parts = job->textData().split(QLatin1Char(' '));
        takeIdentity(parts[0], parts[1]);
    });
    job->start();
}

void MatrixManager::doLogin(const QString &matrixId, const QString &password)
{
    const auto username = matrixId.split(QLatin1Char(':'))[0].mid(1);
    QUrl const loginUrl(QStringLiteral("%1/_matrix/client/v3/login").arg(m_serverUrl));
    const QJsonDocument data(QJsonObject {
            {QLatin1String("identifier"), QJsonObject{
               {QLatin1String("type"), QLatin1String("m.id.user")},
               {QLatin1String("user"), username},
            }},
        {QLatin1String("initial_device_display_name"), QLatin1String("KDE Itinerary")}, //TODO nicer name
        {QLatin1String("password"), password},
        {QLatin1String("type"), QLatin1String("m.login.password")},
    });
    QNetworkRequest request(loginUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    auto *const reply = m_nam->post(request, data.toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        if (reply->error() != QNetworkReply::NoError) {
            if (reply->error() == QNetworkReply::ContentAccessDenied) {
                setErrorString(i18n("Wrong password"));
            } else {
                setErrorString(i18n("Error logging in: (%1) %2", reply->error(), reply->errorString()));
            }
            return;
        }
        const auto json = QJsonDocument::fromJson(reply->readAll()).object();
        m_accessToken = json[QLatin1String("access_token")].toString();
        m_deviceId = json[QLatin1String("device_id")].toString();
        setUserId(json[QLatin1String("user_id")].toString());
        auto *job = new QKeychain::WritePasswordJob(qAppName());
        job->setAutoDelete(true);
        job->setKey(QStringLiteral("Matrix"));
        job->setTextData(QStringLiteral("%1 %2").arg(m_userId, m_accessToken));
        job->start();
        sync();
    });
}

bool MatrixManager::isLoggedIn() const
{
    return !m_userId.isEmpty();
}

QString MatrixManager::userId() const
{
    return m_userId;
}

void MatrixManager::setUserId(const QString &userId)
{
    m_userId = userId;
    setErrorString({});
    Q_EMIT userIdChanged();
    Q_EMIT isLoggedInChanged();
}

void MatrixManager::logout()
{
    const QUrl logoutUrl(QStringLiteral("%1/_matrix/client/v3/logout").arg(m_serverUrl));
    QNetworkRequest logoutRequest(logoutUrl);
    logoutRequest.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(m_accessToken).toLatin1());
    auto *const reply = m_nam->post(logoutRequest, QByteArray());
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        if (reply->error() != QNetworkReply::NoError) {
            setErrorString(i18n("Unable to log out from server: (%1) %2", reply->error(), reply->errorString()));
            return;
        }
        const auto json = QJsonDocument::fromJson(reply->readAll()).object();
        m_accessToken = QString();
        m_deviceId = QString();
        setUserId(QString());
        auto *job = new QKeychain::DeletePasswordJob(qAppName());
        job->setAutoDelete(true);
        job->setKey(QStringLiteral("Matrix"));
        job->start();
    });
}

void MatrixManager::setErrorString(const QString &errorString)
{
    m_errorString = errorString;
    Q_EMIT errorStringChanged();
}

QString MatrixManager::errorString() const {
    return m_errorString;
}

void MatrixManager::takeIdentity(const QString &matrixId, const QString &accessToken)
{
    const auto homeserver = matrixId.split(QLatin1Char(':'))[1];
    const QUrl wellKnownUrl(QStringLiteral("https://%1/.well-known/matrix/client").arg(homeserver));
    auto *const reply = m_nam->get(QNetworkRequest(wellKnownUrl));
    connect(reply, &QNetworkReply::finished, this, [homeserver, matrixId, accessToken, this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            setErrorString(i18n("Unable to find matrix server %1", homeserver));
            return;
        }
        const auto json = QJsonDocument::fromJson(reply->readAll()).object();
        m_serverUrl = json[QStringLiteral("m.homeserver")][QStringLiteral("base_url")].toString();
        const QUrl whoamiUrl(QStringLiteral("%1/_matrix/client/v3/account/whoami").arg(m_serverUrl));
        QNetworkRequest whoamiRequest(whoamiUrl);
        whoamiRequest.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(accessToken).toLatin1());
        auto *reply = m_nam->get(whoamiRequest);
        connect(reply, &QNetworkReply::finished, this, [=](){
            if (reply->error() != QNetworkReply::NoError) {
                setErrorString(
                        i18n("Unable to restore matrix connection:  (%1) %2", reply->error(), reply->errorString()));
                return;
            }
            const auto json = QJsonDocument::fromJson(reply->readAll()).object();
            m_accessToken = accessToken;
            m_deviceId = json[QStringLiteral("device_id")].toString();
            setUserId(json[QStringLiteral("user_id")].toString());
            sync();
        });
    });
}

void MatrixManager::sync()
{
    auto foo = QStringLiteral("{\"account_data\":{\"limit\":0, \"senders\": []},\"presence\":{\"limit\":0},\"room\":{\"account_data\":{\"limit\":0},\"ephemeral\":{\"limit\":0},\"state\":{\"lazy_load_members\":true},\"timeline\":{\"limit\":0}}}");
    QUrl syncUrl(QStringLiteral("%1/_matrix/client/v3/sync?filter=%2").arg(m_serverUrl, foo));
    QNetworkRequest syncRequest(syncUrl);
    syncRequest.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(m_accessToken).toLatin1());
    auto *reply = m_nam->get(syncRequest);
    connect(reply, &QNetworkReply::finished, this, [=](){
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << reply->error() << reply->errorString();
            return;
        }
        const auto json = QJsonDocument::fromJson(reply->readAll()).object();
        const auto &rooms = json[QStringLiteral("rooms")][QStringLiteral("join")].toObject();
        const auto &keys = rooms.keys();
        for (const auto &key : keys) {
            qWarning() << key;
        }
    });
}
