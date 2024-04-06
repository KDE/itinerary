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
#include <QXmlStreamReader>

OnlineTicketRetrievalJob::OnlineTicketRetrievalJob(const QString &sourceId, const QVariantMap &arguments, QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
    , m_nam(nam)
{
    QNetworkReply *reply = nullptr;

    // TODO this should be done more modular eventually, similar to the extraction side
    // e.g. by using some form of request templates, or by service-specific scripts
    if (sourceId == QLatin1StringView("db")) {
        const auto ref = arguments.value(QLatin1StringView("reference")).toString();
        if (ref.size() == 6) {
            dbRequestOrderDetails(arguments);
        } else {
            dbRequestFindOrder(arguments);
        }
        return;
    }
    if (sourceId == QLatin1StringView("sncf")) {
        // based on https://www.sncf-connect.com/app/trips/search and stripped to the bare minimum that works
        QNetworkRequest req(QUrl(QStringLiteral("https://www.sncf-connect.com/bff/api/v1/trips/trips-by-criteria")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("application/json"));
        req.setHeader(QNetworkRequest::UserAgentHeader, QByteArray("Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/111.0"));
        req.setRawHeader("Accept", "application/json, text/plain, */*");
        req.setRawHeader("x-bff-key", "ah1MPO-izehIHD-QZZ9y88n-kku876");
        QByteArray postData("{\"reference\":\"" + arguments.value(QLatin1StringView("reference")).toString().toUtf8() + "\",\"name\":\"" + arguments.value(QLatin1StringView("name")).toString().toUtf8() + "\"}");
        reply = nam->post(req, postData);
        reply->setParent(this);
        connect(reply, &QNetworkReply::finished, this, [reply, this]() { handleReply(reply); });
        return;
    }

    QMetaObject::invokeMethod(this, &OnlineTicketRetrievalJob::finished, Qt::QueuedConnection);
}

OnlineTicketRetrievalJob::~OnlineTicketRetrievalJob() = default;

QList<QVariant> OnlineTicketRetrievalJob::result() const { return m_result; }

QString OnlineTicketRetrievalJob::errorMessage() const
{
    return m_errorMsg;
}

void OnlineTicketRetrievalJob::dbRequestFindOrder(const QVariantMap &arguments)
{
    QNetworkRequest req(QUrl(QStringLiteral("https://fahrkarten.bahn.de/mobile/dbc/xs.go?")));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("application/x-www-form-urlencoded"));
    QByteArray postData(R"(<rqfindorder version="1.0"><rqheader v="23080000" os="KCI" app="NAVIGATOR"/><rqorder on=")"
        + arguments.value(QLatin1StringView("reference")).toString().toUtf8()
        + R"("/><authname tln=")"
        + arguments.value(QLatin1StringView("name")).toString().toUtf8()
        + R"("/></rqfindorder>)");
    auto reply = m_nam->post(req, postData);
    reply->setParent(this);
    connect(reply, &QNetworkReply::finished, this, [reply, arguments, this]() {
        reply->deleteLater();
        auto args = arguments;
        args.insert(QLatin1StringView("kwid"), dbParseKwid(reply));
        dbRequestOrderDetails(args);
    });
}

QString OnlineTicketRetrievalJob::dbParseKwid(QIODevice *io)
{
    QXmlStreamReader reader(io);
    while (!reader.atEnd()) {
        reader.readNextStartElement();
        if (const auto kwid = reader.attributes().value(QLatin1StringView("kwid")); !kwid.isEmpty()) {
            return kwid.toString();
        }
    }
    return {};
}

void OnlineTicketRetrievalJob::dbRequestOrderDetails(const QVariantMap &arguments)
{
    const auto kwid = arguments.value(QLatin1StringView("kwid")).toString();

    QNetworkRequest req(QUrl(QStringLiteral("https://fahrkarten.bahn.de/mobile/dbc/xs.go?")));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("application/x-www-form-urlencoded"));
    QByteArray postData(R"(<rqorderdetails version="1.0"><rqheader v="23040000" os="KCI" app="KCI-Webservice"/><rqorder on=")"
        + arguments.value(QLatin1StringView("reference")).toString().toUpper().toUtf8()
        + (kwid.isEmpty() ? QByteArray() : QByteArray(R"(" kwid=")" + kwid.toUtf8()))
        + R"("/><authname tln=")"
        + arguments.value(QLatin1StringView("name")).toString().toUtf8()
        + R"("/></rqorderdetails>)");
    auto reply = m_nam->post(req, postData);
    reply->setParent(this);
    connect(reply, &QNetworkReply::finished, this, [reply, this]() { handleReply(reply); });
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

#include "moc_onlineticketretrievaljob.cpp"
