/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "onlineticketretrievaljob.h"

#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorResult>
#include <KItinerary/HttpResponse>
#include <KItinerary/JsonLdDocument>

#include <QNetworkAccessManager>
#include <QNetworkReply>

OnlineTicketRetrievalJob::OnlineTicketRetrievalJob(const QString &sourceId, const QVariantMap &arguments, QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
{
    QNetworkReply *reply = nullptr;

    // TODO this should be done more modular eventually, similar to the extraction side
    // e.g. by using some form of request templates, or by service-specific scripts
    if (sourceId == QLatin1String("db")) {
        QNetworkRequest req(QUrl(QStringLiteral("https://fahrkarten.bahn.de/mobile/dbc/xs.go?")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("application/x-www-form-urlencoded"));
        QByteArray postData("<rqorderdetails version=\"1.0\"><rqheader v=\"19120000\" os=\"KCI\" app=\"KCI-Webservice\"/><rqorder on=\"" + arguments.value(QLatin1String("reference")).toString().toUpper().toUtf8() + "\"/><authname tln=\"" + arguments.value(QLatin1String("name")).toString().toUtf8() + "\"/></rqorderdetails>");
        reply = nam->post(req, postData);
    } else if (sourceId == QLatin1String("sncf")) {
        // based on https://www.sncf-connect.com/app/trips/search and stripped to the bare minimum that works
        QNetworkRequest req(QUrl(QStringLiteral("https://www.sncf-connect.com/bff/api/v1/trips/trips-by-criteria")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("application/json"));
        req.setHeader(QNetworkRequest::UserAgentHeader, QByteArray("Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/111.0"));
        req.setRawHeader("Accept", "application/json, text/plain, */*");
        req.setRawHeader("x-bff-key", "ah1MPO-izehIHD-QZZ9y88n-kku876");
        QByteArray postData("{\"reference\":\"" + arguments.value(QLatin1String("reference")).toString().toUtf8() + "\",\"name\":\"" + arguments.value(QLatin1String("name")).toString().toUtf8() + "\"}");
        reply = nam->post(req, postData);
    }

    if (reply) {
        reply->setParent(this);
        connect(reply, &QNetworkReply::finished, this, [reply, this]() { handleReply(reply); });
    } else {
        QMetaObject::invokeMethod(this, &OnlineTicketRetrievalJob::finished, Qt::QueuedConnection);
    }
}

OnlineTicketRetrievalJob::~OnlineTicketRetrievalJob() = default;

QVector<QVariant> OnlineTicketRetrievalJob::result() const
{
    return m_result;
}

QString OnlineTicketRetrievalJob::errorMessage() const
{
    return m_errorMsg;
}

void OnlineTicketRetrievalJob::handleReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        m_errorMsg = reply->errorString();
    } else {
        using namespace KItinerary;
        ExtractorEngine engine;
        engine.setContent(HttpResponse::fromNetworkReply(reply), u"internal/http-response");
        m_result = JsonLdDocument::fromJson(engine.extract());
    }
    Q_EMIT finished();
}
