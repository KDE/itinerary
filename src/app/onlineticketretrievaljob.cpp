/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "onlineticketretrievaljob.h"

#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorResult>
#include <KItinerary/HttpResponse>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QXmlStreamReader>

using namespace Qt::Literals;

OnlineTicketRetrievalJob::OnlineTicketRetrievalJob(const QString &sourceId, const QVariantMap &arguments, QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
    , m_nam(nam)
{
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
    if (sourceId == "db-share"_L1) {
        QNetworkRequest req(QUrl("https://int.bahn.de/web/api/angebote/verbindung/"_L1 + arguments.value("uuid"_L1).toString()));
        auto reply = nam->get(req);
        setupReply(reply);
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
        auto reply = nam->post(req, postData);
        setupReply(reply);
        return;
    }

    QMetaObject::invokeMethod(this, &OnlineTicketRetrievalJob::finished, Qt::QueuedConnection);
}

OnlineTicketRetrievalJob::~OnlineTicketRetrievalJob() = default;

QJsonArray OnlineTicketRetrievalJob::result() const { return m_result; }

QString OnlineTicketRetrievalJob::errorMessage() const
{
    return m_errorMsg;
}

void OnlineTicketRetrievalJob::dbRequestFindOrder(const QVariantMap &arguments)
{
    QNetworkRequest req(QUrl(u"https://fahrkarten.bahn.de/mobile/dbc/xs.go?"_s));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("application/x-www-form-urlencoded"));
    QByteArray postData(R"(<rqfindorder version="1.0"><rqheader v="23080000" os="KCI" app="NAVIGATOR"/><rqorder on=")"
        + arguments.value("reference"_L1).toString().toUtf8()
        + R"("/><authname tln=")"
        + arguments.value("name"_L1).toString().toUtf8()
        + R"("/></rqfindorder>)");
    auto reply = m_nam->post(req, postData);
    reply->setParent(this);
    connect(reply, &QNetworkReply::finished, this, [reply, arguments, this]() {
        reply->deleteLater();
        auto args = arguments;
        args.insert("kwid"_L1, dbParseKwid(reply));
        dbRequestOrderDetails(args);
    });
}

QStringList OnlineTicketRetrievalJob::dbParseKwid(QIODevice *io)
{
    QStringList kwids;
    QXmlStreamReader reader(io);
    while (!reader.atEnd()) {
        reader.readNextStartElement();
        if (const auto kwid = reader.attributes().value("kwid"_L1); !kwid.isEmpty()) {
            kwids.push_back(kwid.toString());
        }
    }
    return kwids;
}

void OnlineTicketRetrievalJob::dbRequestOrderDetails(const QVariantMap &arguments)
{
    auto kwids = arguments.value("kwid"_L1).toStringList();
    if (kwids.empty()) {
        kwids.push_back(QString());
    }

    for (const auto &kwid : kwids) {
        QNetworkRequest req(QUrl(u"https://fahrkarten.bahn.de/mobile/dbc/xs.go?"_s));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("application/x-www-form-urlencoded"));
        QByteArray postData(R"(<rqorderdetails version="1.0"><rqheader v="23040000" os="KCI" app="KCI-Webservice"/><rqorder on=")"
            + arguments.value("reference"_L1).toString().toUpper().toUtf8()
            + (kwid.isEmpty() ? QByteArray() : QByteArray(R"(" kwid=")" + kwid.toUtf8()))
            + R"("/><authname tln=")"
            + arguments.value("name"_L1).toString().toUtf8()
            + R"("/></rqorderdetails>)");
        qDebug() << req.url() << postData;
        auto reply = m_nam->post(req, postData);
        setupReply(reply);
    }
}

void OnlineTicketRetrievalJob::setupReply(QNetworkReply *reply)
{
    reply->setParent(this);
    ++m_pendingReplies;
    connect(reply, &QNetworkReply::finished, this, [reply, this]() { handleReply(reply); });
}

void OnlineTicketRetrievalJob::handleReply(QNetworkReply *reply)
{
    --m_pendingReplies;
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        m_errorMsg = reply->errorString();
    } else {
        using namespace KItinerary;
        ExtractorEngine engine;
        engine.setContent(HttpResponse::fromNetworkReply(reply), u"internal/http-response");
        const auto res = engine.extract();
        std::copy(res.begin(), res.end(), std::back_inserter(m_result));
    }
    if (m_pendingReplies <= 0) {
        Q_EMIT finished();
    }
}

#include "moc_onlineticketretrievaljob.cpp"
