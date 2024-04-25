/*
    SPDX-FileCopyrightText: 2018-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "downloadjob.h"
#include "logging.h"
#include "onlineticketretrievaljob.h"
#include "osmimportjob.h"

#include <KItinerary/JsonLdDocument>

#include <KLocalizedString>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

using namespace Qt::Literals::StringLiterals;

DownloadJob::DownloadJob(const QUrl &url, QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
{
    if (handleOsmUrl(url, nam)) {
        return;
    }

    // see if this is a URL patterns for an online ticket
    if (handleOnlineTicketRetrievalUrl(url, nam)) {
        return;
    }

    // try if this provides an ActivityPub object
    auto reply = makeActivityPubRequest(url, nam);
    connect(reply, &QNetworkReply::finished, this, [this, reply, url, nam]() {
        if (handleActivityPubReply(reply)) {
            return;
        }

        // ActivityPub request failed, try regular download instead
        auto reply2 = makeDownloadRequest(url, nam);
        connect(reply2, &QNetworkReply::finished, this, [this, reply2]() { handleDownloadReply(reply2); });
    });
}

DownloadJob::~DownloadJob() = default;

bool DownloadJob::hasError() const
{
    return !m_errorMessage.isEmpty();
}

QByteArray DownloadJob::data() const
{
    return m_data;
}

QString DownloadJob::errorMessage() const
{
    return m_errorMessage;
}

bool DownloadJob::handleOsmUrl(const QUrl &url, QNetworkAccessManager *nam)
{
    if (url.host() != "www.openstreetmap.org"_L1) {
        return false;
    }

    static const QRegularExpression pathRx(uR"(^/(node|way|relation)/(\d+)/?$)"_s);
    const auto pathMatch = pathRx.match(url.path());
    if (!pathMatch.hasMatch()) {
        return false;
    }

    OSM::Type type = OSM::Type::Null;
    if (pathMatch.capturedView(1) == "node"_L1) {
        type = OSM::Type::Node;
    } else if (pathMatch.capturedView(1) == "way"_L1) {
        type = OSM::Type::Way;
    } else if (pathMatch.capturedView(1) == "relation"_L1) {
        type = OSM::Type::Relation;
    } else {
        return false;
    }

    auto job = new OsmImportJob(type, pathMatch.capturedView(2).toLongLong(), nam, this);
    connect(job, &OsmImportJob::finished, this, [this, job]() {
        job->deleteLater();
        if (job->result().isNull()) {
            m_errorMessage = job->errorMessage();
        } else {
            m_data = QJsonDocument(KItinerary::JsonLdDocument::toJson(job->result())).toJson(QJsonDocument::Compact);
        }
        Q_EMIT finished();
    });
    return true;
}

bool DownloadJob::handleOnlineTicketRetrievalUrl(const QUrl &url, QNetworkAccessManager *nam)
{
    // TODO this needs to be gone generically and ideally based on some declarative data
    if (url.host() == "dbnavigator.bahn.de"_L1 && url.path() == "/loadorder"_L1) {
        const auto query = QUrlQuery(url);
        QVariantMap args({
            { "name"_L1, query.queryItemValue("name"_L1).toUpper() },
            { "reference"_L1, query.queryItemValue("on"_L1) }
        });

        qCDebug(Log) << "Doing online ticket retrieval for" << url << args;
        auto job = new OnlineTicketRetrievalJob(u"db"_s, args, nam, this);
        connect(job, &OnlineTicketRetrievalJob::finished, this, [this, job]() {
            job->deleteLater();
            if (job->result().isEmpty()) {
                m_errorMessage = job->errorMessage();
            } else {
                m_data = QJsonDocument(KItinerary::JsonLdDocument::toJson(job->result())).toJson(QJsonDocument::Compact);
            }
            Q_EMIT finished();
        });
        return true;
    }
    return false;
}

QNetworkReply* DownloadJob::makeActivityPubRequest(QUrl url, QNetworkAccessManager *nam)
{
    url.setScheme("https"_L1);
    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    req.setRawHeader("Accept", "application/ld+json; profile=\"https://www.w3.org/ns/activitystreams\"");
    qCDebug(Log) << "Checking for ActivityStreams" << url;
    auto reply = nam->get(req);
    reply->setParent(this);
    return reply;
}

bool DownloadJob::handleActivityPubReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NetworkSessionFailedError) {
        // assumed persistent network error, not worth trying a regular download either
        m_errorMessage = i18n("Network error: %1", reply->errorString());
        Q_EMIT finished();
        return true;
    }
    if (reply->error() != QNetworkReply::NoError) {
        // ActivityPub not available, try regular download
        qCDebug(Log) << reply->url() << reply->errorString();
        return false;
    }

    m_data = reply->readAll();
    Q_EMIT finished();
    return true;
}

QNetworkReply* DownloadJob::makeDownloadRequest(QUrl url, QNetworkAccessManager *nam)
{
    url.setScheme("https"_L1);
    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    qCDebug(Log) << "Downloading" << url;
    auto reply = nam->get(req);
    reply->setParent(this);
    return reply;
}

void DownloadJob::handleDownloadReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qCDebug(Log) << reply->url() << reply->errorString();
        m_errorMessage = i18n("Download failed: %1", reply->errorString());
    } else {
        m_data = reply->readAll();
    }
    Q_EMIT finished();
}

#include "moc_downloadjob.cpp"
