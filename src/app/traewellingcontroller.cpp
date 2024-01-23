// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "traewellingcontroller.h"

#include <QDebug>
#include <QDesktopServices>
#include <QCryptographicHash>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QTcpServer>
#include <QHostAddress>
#include <QRegularExpression>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QCoreApplication>
#include <QRandomGenerator>

#include <qt6keychain/keychain.h>

static auto nam = new QNetworkAccessManager();

static auto baseUrl = QStringLiteral("https://traewelling.de");

static QUrl buildUrl(const QString &endpoint)
{
    return QUrl(baseUrl + endpoint);
}

static void writeKeychain(const QString &key, const QString &value)
{
    QKeychain::WritePasswordJob job(qAppName());
    job.setBinaryData(value.toLatin1());
    job.setKey(key);
    QEventLoop loop;
    QObject::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();
}

static std::optional<QString> readKeychain(const QString &key)
{
    QKeychain::ReadPasswordJob job(qAppName());
    job.setKey(key);
    QEventLoop loop;
    QObject::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();
    if (job.error() != QKeychain::NoError) {
        return std::nullopt;
    }
    return QString::fromLatin1(job.binaryData());
}

static void deleteKeychain(const QString &key)
{
    QKeychain::DeletePasswordJob job(qAppName());
    job.setKey(key);
    QEventLoop loop;
    QObject::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();
}

TraewellingController::TraewellingController(QObject *parent)
    : QObject(parent)
{
    const auto accessToken = readKeychain(QStringLiteral("traewelling-access"));
    const auto refreshToken = readKeychain(QStringLiteral("traewelling-refresh"));

    if (accessToken) {
        setAccessToken(*accessToken);
        auto tokenUrl = buildUrl(QStringLiteral("/oauth/token"));
        auto request = QNetworkRequest(buildUrl(QStringLiteral("/api/v1/auth/refresh")));
        request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(*accessToken).toLatin1());
        auto reply = nam->post(request, QByteArray());
        connect(reply, &QNetworkReply::finished, this, [this, reply, tokenUrl, refreshToken]() {
            if (reply->error() != QNetworkReply::NoError) {
                QUrlQuery query;
                query.addQueryItem(QLatin1String("grant_type"), QStringLiteral("refresh_token"));
                query.addQueryItem(QLatin1String("refresh_token"), *refreshToken);
                query.addQueryItem(QLatin1String("redirect_uri"), QString::fromLatin1(QUrl::toPercentEncoding(QStringLiteral("http://127.0.0.1:11450"))));
                query.addQueryItem(QLatin1String("client_id"), QLatin1String("98"));
                QNetworkRequest refreshRequest(tokenUrl);
                refreshRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                    QStringLiteral("application/x-www-form-urlencoded"));
                auto refreshReply = nam->post(refreshRequest, query.toString(QUrl::FullyEncoded).toUtf8());
                connect(refreshReply, &QNetworkReply::finished, this, [this, refreshReply](){
                    if (refreshReply->error() != QNetworkReply::NoError) {
                        logout(false);
                        return;
                    }
                    auto json = QJsonDocument::fromJson(refreshReply->readAll()).object();
                    writeKeychain(QStringLiteral("traewelling-refresh"), json[QStringLiteral("refresh_token")].toString());
                    setAccessToken(json[QStringLiteral("access_token")].toString());
                    writeKeychain(QStringLiteral("traewelling-access"), m_accessToken);
                });
            } else {
                auto json = QJsonDocument::fromJson(reply->readAll()).object();
                setAccessToken(json[QStringLiteral("data")][QStringLiteral("token")].toString());
                writeKeychain(QStringLiteral("traewelling-access"), m_accessToken);
                loadData();
            }
        });

    }
}

void TraewellingController::get(const QString &endpoint, const std::function<void(QJsonObject)> &then)
{
    auto request = QNetworkRequest(buildUrl(endpoint));
    request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(m_accessToken).toLatin1());
    auto reply = nam->get(request);
    connect(reply, &QNetworkReply::finished, this, [then, reply](){
        auto json = QJsonDocument::fromJson(reply->readAll()).object();
        then(json);
    });
}

void TraewellingController::post(const QString &endpoint, const QByteArray &data, const std::function<void(QJsonObject)> &then)
{
    auto request = QNetworkRequest(buildUrl(endpoint));
    request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(m_accessToken).toLatin1());
    auto reply = nam->post(request, data);
    connect(reply, &QNetworkReply::finished, this, [then, reply](){
        auto json = QJsonDocument::fromJson(reply->readAll()).object();
        then(json);
    });
}

void TraewellingController::loadData()
{
    get(QLatin1String("/api/v1/auth/user"), [this](const QJsonObject &json){
        setUsername(json[QStringLiteral("data")][QStringLiteral("username")].toString());
    });
}

void TraewellingController::login()
{
    m_state = QString::number(QRandomGenerator::securelySeeded().generate64());
    m_verifier = QCryptographicHash::hash(QStringLiteral("Random %1").arg(QRandomGenerator::securelySeeded().generate64()).toLatin1(), QCryptographicHash::Sha256).toHex();
    auto challenge = QString::fromLatin1(QCryptographicHash::hash(m_verifier, QCryptographicHash::Sha256).toBase64());
    auto challengeString = challenge.replace(QLatin1Char('+'), QLatin1Char('-')).replace(QLatin1Char('/'), QLatin1Char('_')).left(challenge.indexOf(QLatin1Char('=')));
    QUrlQuery query;
    query.addQueryItem(QLatin1String("code_challenge"), challengeString);
    query.addQueryItem(QStringLiteral("code_challenge_method"), QLatin1String("S256"));
    query.addQueryItem(QStringLiteral("client_id"), QLatin1String("98"));
    query.addQueryItem(QStringLiteral("redirect_uri"), QLatin1String("http://127.0.0.1:11450"));
    query.addQueryItem(QStringLiteral("response_type"), QLatin1String("code"));
    query.addQueryItem(QStringLiteral("scope"), QLatin1String("write-statuses read-settings read-settings-profile"));
    query.addQueryItem(QLatin1String("state"), m_state);
    auto authUrl = buildUrl(QStringLiteral("/oauth/authorize"));
    authUrl.setQuery(query);

    QDesktopServices::openUrl(authUrl);
    qWarning() << "Open" << authUrl.toString();

    auto server = new QTcpServer(this);
    server->listen(QHostAddress(QStringLiteral("127.0.0.1")), 11450);
    connect(server, &QTcpServer::newConnection, this, [this, server]() {
        auto connection = server->nextPendingConnection();
        connect(connection, &QIODevice::readyRead, this, [this, connection]() {
            auto data = QString::fromLatin1(connection->readAll());
            QRegularExpression codeRegex(QStringLiteral("code=([a-f0-9]+)&"));
            auto code = codeRegex.match(data).captured(1);
            QRegularExpression stateRegex(QStringLiteral("state=([0-9]+)"));
            connection->write("HTTP/1.0 200 OK\r\n\r\nYou can return to Itinerary now.");
            connection->close();
            if (stateRegex.match(data).captured(1) != m_state) {
                return;
            }
            auto tokenUrl = buildUrl(QStringLiteral("/oauth/token"));
            QUrlQuery query;
            query.addQueryItem(QLatin1String("grant_type"), QStringLiteral("authorization_code"));
            query.addQueryItem(QLatin1String("code"), QString::fromLatin1(QUrl::toPercentEncoding(code)));
            query.addQueryItem(QLatin1String("redirect_uri"), QString::fromLatin1(QUrl::toPercentEncoding(QStringLiteral("http://127.0.0.1:11450"))));
            query.addQueryItem(QLatin1String("code_verifier"), QString::fromLatin1(m_verifier));
            query.addQueryItem(QLatin1String("client_id"), QLatin1String("98"));
            QNetworkRequest request(tokenUrl);
            request.setHeader(QNetworkRequest::ContentTypeHeader,
                QStringLiteral("application/x-www-form-urlencoded"));
            auto reply = nam->post(request, query.toString(QUrl::FullyEncoded).toUtf8());
            connect(reply, &QNetworkReply::finished, this, [this, reply](){
                auto json = QJsonDocument::fromJson(reply->readAll()).object();
                setAccessToken(json[QLatin1String("access_token")].toString());
                const auto refreshToken = json[QLatin1String("refresh_token")].toString();
                writeKeychain(QStringLiteral("traewelling-access"), m_accessToken);
                writeKeychain(QStringLiteral("traewelling-refresh"), refreshToken);
                loadData();
            });
        });
    });
}

void TraewellingController::logout(bool server)
{
    if (server) {
        post(QStringLiteral("/api/v1/auth/logout"), {}, [](const auto &){});
    }
    setAccessToken({});
    setUsername({});
    deleteKeychain(QStringLiteral("traewelling-access"));
    deleteKeychain(QStringLiteral("traewelling-refresh"));
}

QString TraewellingController::username() const
{
    return m_username;
}

void TraewellingController::setUsername(const QString &username)
{
    m_username = username;
    Q_EMIT usernameChanged();
}

void TraewellingController::setAccessToken(const QString &accessToken)
{
    m_accessToken = accessToken;
    Q_EMIT isLoggedInChanged();
}

bool TraewellingController::isLoggedIn() const
{
    return m_accessToken.length() > 0;
}

