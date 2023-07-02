/*
    SPDX-FileCopyrightText: 2018-2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "downloadjob.h"
#include "logging.h"
#include "onlineticketretrievaljob.h"

#include <KItinerary/JsonLdDocument>

#include <KLocalizedString>

#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

DownloadJob::DownloadJob(QUrl url, QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
{
    if (handleOnlineTicketRetrievalUrl(url, nam)) {
        return;
    }

    url.setScheme(QLatin1String("https"));
    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    qCDebug(Log) << "Downloading" << url;
    auto reply = nam->get(req);
    reply->setParent(this);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qCDebug(Log) << reply->url() << reply->errorString();
            m_errorMessage = i18n("Download failed: %1", reply->errorString());
        } else {
            m_data = reply->readAll();
        }
        Q_EMIT finished();
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

bool DownloadJob::handleOnlineTicketRetrievalUrl(const QUrl &url, QNetworkAccessManager *nam)
{
    // TODO this needs to be gone generically and ideally based on some declarative data
    if (url.host() == QLatin1String("dbnavigator.bahn.de") && url.path() == QLatin1String("/loadorder")) {
        const auto query = QUrlQuery(url);
        QVariantMap args({
            { QLatin1String("name"), query.queryItemValue(QLatin1String("name")).toUpper() },
            { QLatin1String("reference"), query.queryItemValue(QLatin1String("on")) }
        });

        qCDebug(Log) << "Doing online ticket retrieval for" << url << args;
        auto job = new OnlineTicketRetrievalJob(QStringLiteral("db"), args, nam, this);
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
