// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "deutschbahnaccount.h"
#include "qmlsingletons.h"

#include <QDesktopServices>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QAction>
#include <QNetworkReply>
#include <QRandomGenerator>
#include <QJsonArray>
#include <QUrlQuery>
#include <QtConcurrent>
#include <Prison/ImageScanner>
#include <Prison/ScanResult>

#include <KSharedConfig>
#include <KLocalizedString>

#include <qt6keychain/keychain.h>

using namespace Qt::StringLiterals;

static constexpr auto clientIdentifier = "kf_mobile"_L1;
static constexpr auto redirectUrl = "dbnav://dbnavigator.bahn.de/auth"_L1;
static constexpr auto accessTokenUrl = "https://accounts.bahn.de/auth/realms/db/protocol/openid-connect/token"_L1;
static constexpr auto authorizationUrl = "https://accounts.bahn.de/auth/realms/db/protocol/openid-connect/auth"_L1;

DeutschBahnAccount::DeutschBahnAccount(QObject *parent)
    : AbstractAccount(parent)
    , m_api(QUrl(u"https://app.vendo.noncd.db.de/mob"_s))
    , m_syncAction(new QAction(QIcon::fromTheme(u"state-sync-symbolic"_s), i18nc("@action:button", "Synchronize"), this))
{
    m_syncAction->setEnabled(false);

    connect(m_syncAction, &QAction::triggered, this, &DeutschBahnAccount::downloadTickets);
}

QString DeutschBahnAccount::accountName() const
{
    return m_accountName;
}

void DeutschBahnAccount::saveAccount(KConfigGroup &configGroup) const
{
    configGroup.writeEntry("accountName", m_accountName);
    configGroup.writeEntry("accountId", m_accountId);
}

void DeutschBahnAccount::loadAccount(const KConfigGroup &configGroup)
{
    m_accountName = configGroup.readEntry("accountName");
    Q_EMIT void accountNameChanged();

    m_accountId = configGroup.readEntry("accountId");

    auto job = new QKeychain::ReadPasswordJob(u"org.kde.itinerary"_s, this);
    job->setKey(m_identifier + u'-' + m_accountId);
    connect(job, &QKeychain::ReadPasswordJob::finished, this, [this, job] {
        if (job->error()) {
            qWarning() << "Could not read password" << job->errorString();
        }
        const auto doc = QJsonDocument::fromJson(job->binaryData());
        if (doc.isNull()) {
            qWarning() << "invalid data";
            return;
        }
        const auto obj = doc.object();
        const auto expirationDate = QDateTime::fromString(obj["expires_at"_L1].toString());
        if (expirationDate < QDateTime::currentDateTime()) {
            setupOauth();
            m_oauth->setRefreshToken(obj["refresh_token"_L1].toString());
            m_oauth->refreshTokens();
			return;
        }

        m_api.setBearerToken(obj["token"_L1].toString().toUtf8());
        m_syncAction->setEnabled(true);
    });
    job->start();
}

void DeutschBahnAccount::setupOauth()
{
    if (m_oauth) {
        return;
    }
    m_oauth = new QOAuth2AuthorizationCodeFlow(m_namFactory(), this);
    const QSet<QByteArray> scopes{"offline_access"};

    m_oauth->setAuthorizationUrl(QUrl(authorizationUrl));
    m_oauth->setTokenUrl(QUrl(accessTokenUrl));
    m_oauth->setClientIdentifier(clientIdentifier);
    m_oauth->setRequestedScopeTokens(scopes);
    m_oauth->setPkceMethod(QOAuth2AuthorizationCodeFlow::PkceMethod::S256);
    m_oauth->setAutoRefresh(true);

    m_handler.setRedirectUrl(QUrl{redirectUrl});
    m_oauth->setReplyHandler(&m_handler);

    connect(m_oauth, &QAbstractOAuth::authorizeWithBrowser, this, [](const QUrl &url) {
        QDesktopServices::openUrl(url);
    });
    connect(m_oauth, &QAbstractOAuth::granted, this, [this]() {
        const auto token = m_oauth->token();

        std::error_code error;
        const auto jwt = token.split(u'.');
        auto doc = QJsonDocument::fromJson(QByteArray::fromBase64(jwt[1].toUtf8()));

        const auto object = doc.object();

        m_accountId = object["kundenkontoid"_L1].toString();
        m_accountName = object["email"_L1].toString();
        Q_EMIT accountNameChanged();

        writeKeychain(m_identifier + u'-' + m_accountId, QJsonObject{
            { "kundenkontoid"_L1, m_accountId },
            { "token"_L1, m_oauth->token() },
            { "refresh_token"_L1, m_oauth->refreshToken() },
            { "expires_at"_L1, m_oauth->expirationAt().toString() },
        });

        Q_EMIT changed(m_accountId);
        m_syncAction->setEnabled(true);

        // Here we use QNetworkRequestFactory to store the access token
        m_api.setBearerToken(m_oauth->token().toUtf8());

        m_handler.close();
    });
}

void DeutschBahnAccount::writeKeychain(const QString &key, const QJsonObject &value)
{
    auto job = new QKeychain::WritePasswordJob(u"org.kde.itinerary"_s, this);
    job->setBinaryData(QJsonDocument(value).toJson(QJsonDocument::Compact));
    job->setKey(key);
    job->start();
}

void DeutschBahnAccount::startOAuth()
{
    setupOauth();

    // Initiate the authorization
    if (m_handler.listen()) {
        m_oauth->grant();
    }
}

bool DeutschBahnAccount::handleCallback(const QUrl &url)
{
    m_handler.handleAuthorizationRedirect(url);
    return true;
}

QList<QAction *> DeutschBahnAccount::actions() const
{
    return {
        m_syncAction,
    };
}

static QByteArray tokenHex(int byteCount = 16)
{
    QByteArray bytes(byteCount, Qt::Uninitialized);
    for(int i = 0; i < byteCount; ++i) {
        bytes[i] = static_cast<char>(QRandomGenerator::system()->generate() & 0xFF);
    }
    return bytes.toHex();
}

void DeutschBahnAccount::downloadTickets()
{
    auto request = m_api.createRequest(u"kundenkonten/"_s + m_accountId);
    request.setRawHeader("Accept", "application/x.db.vendo.mob.kundenkonto.v7+json");
    request.setRawHeader("X-Correlation-ID", tokenHex());
    auto reply = m_namFactory()->post(request, nullptr);
    connect(reply, &QNetworkReply::finished, this, [reply, request, this]{
        reply->deleteLater();

        if (reply->error()) {
            qWarning() << "Failed" << request.url() << reply->errorString();
            return;
        }

        auto doc = QJsonDocument::fromJson(reply->readAll());

        qWarning() << doc;

        const auto object = doc.object();
        const auto kundenprofils = object["kundenprofile"_L1].toArray();

        for (const auto &profile : kundenprofils) {
            const auto profileObject = profile.toObject();
            const auto id = profileObject["id"_L1].toString();

            auto request = m_api.createRequest(u"reisenuebersicht"_s, QUrlQuery{
                {"kundenprofilId"_L1, id},
            });
            request.setRawHeader("Accept", "application/x.db.vendo.mob.reisenuebersicht.v5+json");
            request.setRawHeader("X-Correlation-ID", tokenHex());

            auto reply = m_namFactory()->get(request);
            connect(reply, &QNetworkReply::finished, this, [reply, request, this]{
                reply->deleteLater();

                if (reply->error()) {
                    qWarning() << "Failed" << request.url() << reply->errorString();
                    return;
                }

                auto doc = QJsonDocument::fromJson(reply->readAll());
                const auto object = doc.object();

                const auto auftragsIndizes = object["auftragsIndizes"_L1].toArray();
                for (const auto &auftragsIndize : auftragsIndizes) {
                    const auto object = auftragsIndize.toObject();

                    const auto auftragsnummer = object["auftragsnummer"_L1].toString();

                    for (const auto &kundenwunschId : object["kundenwunschIds"_L1].toArray()) {

                        auto request = m_api.createRequest(u"auftrag/"_s + auftragsnummer + u"/kundenwunsch/"_s + kundenwunschId.toString());
                        request.setRawHeader("Accept", "application/x.db.vendo.mob.auftraege.v9+json");
                        request.setRawHeader("X-Correlation-ID", tokenHex());

                        auto reply = m_namFactory()->get(request);
                        connect(reply, &QNetworkReply::finished, this, [reply, request, this]{
                            reply->deleteLater();

                            const auto object = QJsonDocument::fromJson(reply->readAll()).object();

							const auto reise = object["reise"_L1].toObject();
							const auto reiseInfos = reise["reiseInfos"_L1].toObject();
							const auto ticket1 = reiseInfos["ticket"_L1].toObject();
							const auto anzeige = ticket1["anzeige"_L1].toObject();
							const auto gueltigkeitAb = QDateTime::fromString(anzeige["gueltigkeitAb"_L1].toString());

							const auto ticket2 = ticket1["ticket"_L1].toString();
							const auto html = QByteArray::fromBase64(ticket2.toUtf8());

							// HTML parsing to extract img data with a regex :)
							QRegularExpression re(u"data:image/png;base64,([^\"]*)\""_s);
							auto matchIterator = re.globalMatch(QString::fromUtf8(html));
							while (matchIterator.hasNext()) {
								auto match = matchIterator.next();

								QByteArray imageData = QByteArray::fromBase64(match.capturedView(1).toUtf8());
								QImage img;
								img.loadFromData(imageData, "PNG");

								QtConcurrent::run([&img]() {
									return Prison::ImageScanner::scan(img, Prison::Format::Aztec);
								}).then([img](const Prison::ScanResult &result) {
									if (result.hasBinaryData()) {
										ImportControllerInstance::instance->importData(result.binaryData());
									}
								});
							}
                        });
                    }
                }
            });
        }
    });
}
