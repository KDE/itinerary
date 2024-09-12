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
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QCoreApplication>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QSettings>

#include <qt6keychain/keychain.h>

#include "applicationcontroller.h"

static auto baseUrl = QStringLiteral("https://traewelling.de");

static QUrl buildUrl(const QString &endpoint, const QUrlQuery &query)
{
    QUrl url(baseUrl + endpoint);
    url.setQuery(query);
    return url;
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

TraewellingController::TraewellingController(std::function<QNetworkAccessManager*()> namFactory, QObject *parent)
    : QObject(parent)
    , m_namFactory(std::move(namFactory))
{
    QSettings settings;
    if (!settings.value(QStringLiteral("TraewellingEnabled"), false).toBool()) {
        return;
    }
    const auto accessToken = readKeychain(QStringLiteral("traewelling-access"));
    const auto refreshToken = readKeychain(QStringLiteral("traewelling-refresh"));

    if (accessToken) {
        setAccessToken(*accessToken);
        auto tokenUrl = buildUrl(QStringLiteral("/oauth/token"), {});
        auto request = QNetworkRequest(buildUrl(QStringLiteral("/api/v1/auth/refresh"), {}));
        request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(*accessToken).toLatin1());
        request.setHeader(QNetworkRequest::UserAgentHeader, ApplicationController::userAgent());
        auto reply = m_namFactory()->post(request, QByteArray());
        connect(reply, &QNetworkReply::finished, this, [this, reply, tokenUrl, refreshToken]() {
            if (reply->error() != QNetworkReply::NoError) {
                QUrlQuery query;
                query.addQueryItem(QLatin1StringView("grant_type"), QStringLiteral("refresh_token"));
                query.addQueryItem(QLatin1StringView("refresh_token"), *refreshToken);
                query.addQueryItem(QLatin1StringView("redirect_uri"), QString::fromLatin1(QUrl::toPercentEncoding(QStringLiteral("http://127.0.0.1:11450"))));
                query.addQueryItem(QLatin1StringView("client_id"), QLatin1StringView("98"));
                QNetworkRequest refreshRequest(tokenUrl);
                refreshRequest.setHeader(QNetworkRequest::UserAgentHeader, ApplicationController::userAgent());
                refreshRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                    QStringLiteral("application/x-www-form-urlencoded"));
                auto refreshReply = m_namFactory()->post(refreshRequest, query.toString(QUrl::FullyEncoded).toUtf8());
                connect(refreshReply, &QNetworkReply::finished, this, [this, refreshReply](){
                    if (refreshReply->error() != QNetworkReply::NoError) {
                        logout(false);
                        return;
                    }
                    auto json = QJsonDocument::fromJson(refreshReply->readAll()).object();
                    writeKeychain(QStringLiteral("traewelling-refresh"), json[QStringLiteral("refresh_token")].toString());
                    setAccessToken(json[QStringLiteral("access_token")].toString());
                    writeKeychain(QStringLiteral("traewelling-access"), m_accessToken);
                    refreshReply->deleteLater();
                });
            } else {
                auto json = QJsonDocument::fromJson(reply->readAll()).object();
                setAccessToken(json[QStringLiteral("data")][QStringLiteral("token")].toString());
                writeKeychain(QStringLiteral("traewelling-access"), m_accessToken);
                loadData();
            }
            reply->deleteLater();
        });

    }
}

void TraewellingController::get(const QString &endpoint, const QUrlQuery &query, const std::function<void(QJsonObject)> &then)
{
    auto request = QNetworkRequest(buildUrl(endpoint, query));
    request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(m_accessToken).toLatin1());
    request.setHeader(QNetworkRequest::UserAgentHeader, ApplicationController::userAgent());
    auto reply = m_namFactory()->get(request);
    connect(reply, &QNetworkReply::finished, this, [then, reply](){
        auto json = QJsonDocument::fromJson(reply->readAll()).object();
        then(json);
        reply->deleteLater();
    });
}

void TraewellingController::post(const QString &endpoint, const QByteArray &data, const std::function<void(QJsonObject, QNetworkReply::NetworkError)> &then)
{
    auto request = QNetworkRequest(buildUrl(endpoint, {}));
    request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(m_accessToken).toLatin1());
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setHeader(QNetworkRequest::UserAgentHeader, ApplicationController::userAgent());
    auto reply = m_namFactory()->post(request, data);
    connect(reply, &QNetworkReply::finished, this, [then, reply](){
        auto json = QJsonDocument::fromJson(reply->readAll()).object();
        then(json, reply->error());
        reply->deleteLater();
    });
}

void TraewellingController::loadData()
{
    get(QLatin1StringView("/api/v1/auth/user"), {}, [this](const QJsonObject &json){
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
    query.addQueryItem(QLatin1StringView("code_challenge"), challengeString);
    query.addQueryItem(QStringLiteral("code_challenge_method"), QLatin1StringView("S256"));
    query.addQueryItem(QStringLiteral("client_id"), QLatin1StringView("98"));
    query.addQueryItem(QStringLiteral("redirect_uri"), QLatin1StringView("http://127.0.0.1:11450"));
    query.addQueryItem(QStringLiteral("response_type"), QLatin1StringView("code"));
    query.addQueryItem(QStringLiteral("scope"), QLatin1StringView("write-statuses read-settings read-settings-profile"));
    query.addQueryItem(QLatin1StringView("state"), m_state);
    auto authUrl = buildUrl(QStringLiteral("/oauth/authorize"), {});
    authUrl.setQuery(query);

    QDesktopServices::openUrl(authUrl);

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
            auto tokenUrl = buildUrl(QStringLiteral("/oauth/token"), {});
            QUrlQuery query;
            query.addQueryItem(QLatin1StringView("grant_type"), QStringLiteral("authorization_code"));
            query.addQueryItem(QLatin1StringView("code"), QString::fromLatin1(QUrl::toPercentEncoding(code)));
            query.addQueryItem(QLatin1StringView("redirect_uri"), QString::fromLatin1(QUrl::toPercentEncoding(QStringLiteral("http://127.0.0.1:11450"))));
            query.addQueryItem(QLatin1StringView("code_verifier"), QString::fromLatin1(m_verifier));
            query.addQueryItem(QLatin1StringView("client_id"), QLatin1StringView("98"));
            QNetworkRequest request(tokenUrl);
            request.setHeader(QNetworkRequest::UserAgentHeader, ApplicationController::userAgent());
            request.setHeader(QNetworkRequest::ContentTypeHeader,
                QStringLiteral("application/x-www-form-urlencoded"));
            auto reply = m_namFactory()->post(request, query.toString(QUrl::FullyEncoded).toUtf8());
            connect(reply, &QNetworkReply::finished, this, [this, reply](){
                auto json = QJsonDocument::fromJson(reply->readAll()).object();
                setAccessToken(json[QLatin1StringView("access_token")].toString());
                const auto refreshToken = json[QLatin1StringView("refresh_token")].toString();
                writeKeychain(QStringLiteral("traewelling-access"), m_accessToken);
                writeKeychain(QStringLiteral("traewelling-refresh"), refreshToken);
                QSettings settings;
                settings.setValue("TraewellingEnabled", true);
                loadData();
                reply->deleteLater();
            });
        });
    });
}

void TraewellingController::logout(bool server)
{
    if (server) {
        post(QStringLiteral("/api/v1/auth/logout"), {}, [](const auto &, auto){});
    }
    QSettings settings;
    settings.setValue("TraewellingEnabled", false);
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

void TraewellingController::checkin(const QString &departureStationName, const QString &destinationStationName, const QDateTime &departureTime, const QDateTime &arrivalTime, const QString &directionStationName)
{
    get(QStringLiteral("/api/v1/trains/station/autocomplete/%1").arg(departureStationName), {}, [=, this](const QJsonObject &data) {
        const auto array = data[QStringLiteral("data")].toArray();
        if (array.size() == 0) {
            //TODO: Error, no station found
            return;
        }
        const auto departureId = data[QStringLiteral("data")].toArray()[0][QStringLiteral("id")].toInt();

        get(QStringLiteral("/api/v1/trains/station/autocomplete/%1").arg(destinationStationName), {}, [=, this](const QJsonObject &data) {
            const auto array = data[QStringLiteral("data")].toArray();
            if (array.size() == 0) {
                //TODO: Error, no station found
                return;
            }
            const auto arrivalId = data[QStringLiteral("data")].toArray()[0][QStringLiteral("id")].toInt();
            QUrlQuery query;
            query.addQueryItem(QStringLiteral("when"), departureTime.toTimeZone(QTimeZone::UTC).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzzZ")));
            get(QStringLiteral("/api/v1/station/%1/departures").arg(departureId), query, [=, this](const QJsonObject &data) {
                QString tripId;
                QString lineName;
                for (const auto &t : data[QStringLiteral("data")].toArray()) {
                    auto train = t.toObject();
                    if (train[QStringLiteral("direction")].toString() == directionStationName) {
                        tripId = train[QStringLiteral("tripId")].toString();
                        lineName = train[QStringLiteral("line")][QStringLiteral("name")].toString();
                        break;
                    }
                }
                QJsonObject request {
                    {QStringLiteral("body"), QStringLiteral("Traewelling with KDE Itinerary")},
                    {QStringLiteral("business"), 0},
                    {QStringLiteral("visibility"), 0},
                    {QStringLiteral("eventId"), QJsonValue::Null},
                    {QStringLiteral("toot"), false},
                    {QStringLiteral("chainPost"), false},
                    {QStringLiteral("ibnr"), false},
                    {QStringLiteral("toot"), false},
                    {QStringLiteral("tripId"), tripId},
                    {QStringLiteral("lineName"), lineName},
                    {QStringLiteral("start"), departureId},
                    {QStringLiteral("destination"), arrivalId},
                    {QStringLiteral("departure"), departureTime.toTimeZone(QTimeZone::UTC).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzzZ"))},
                    {QStringLiteral("arrival"), arrivalTime.toTimeZone(QTimeZone::UTC).toString(QStringLiteral("yyyy-MM-ddThh:mm:ss.zzzZ"))},
                    {QStringLiteral("force"), false},
                };

                post(QStringLiteral("/api/v1/trains/checkin"), QJsonDocument(request).toJson(QJsonDocument::Compact), [this](const auto &, auto error) {
                    Q_EMIT uploadStatus(error == QNetworkReply::NoError || error == QNetworkReply::ContentConflictError ? Success : Error);
                });
            });
        });
    });
}
