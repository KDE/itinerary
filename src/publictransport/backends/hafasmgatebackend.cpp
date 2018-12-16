/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "hafasmgatebackend.h"
#include "hafasmgateparser.h"
#include "logging.h"

#include <KPublicTransport/Departure>
#include <KPublicTransport/DepartureReply>
#include <KPublicTransport/DepartureRequest>
#include <KPublicTransport/Location>

#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaEnum>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

using namespace KPublicTransport;

HafasMgateBackend::HafasMgateBackend() = default;

bool HafasMgateBackend::queryJourney(JourneyReply *reply, QNetworkAccessManager *nam) const
{
    return false;
}

bool HafasMgateBackend::queryDeparture(DepartureReply *reply, QNetworkAccessManager *nam) const
{
    const auto request = reply->request();
    const auto id = request.stop().identifier(QLatin1String("hafasId")); // ### temporary, until we have proper name lookup
    if (id.isEmpty()) {
        return false;
    }

    QJsonObject top;
    {
        QJsonObject auth;
        auth.insert(QLatin1String("aid"), m_aid);
        auth.insert(QLatin1String("type"), QLatin1String("AID"));
        top.insert(QLatin1String("auth"), auth);
    }
    {
        QJsonObject client;
        client.insert(QLatin1String("id"), m_clientId);
        client.insert(QLatin1String("type"), m_clientType);
        if (!m_clientVersion.isEmpty()) {
            client.insert(QLatin1String("v"), m_clientVersion);
        }
        if (!m_clientName.isEmpty()) {
            client.insert(QLatin1String("name"), m_clientName);
        }
        top.insert(QLatin1String("client"), client);
    }
    top.insert(QLatin1String("formatted"), false);
    top.insert(QLatin1String("lang"), QLatin1String("eng"));
    {
        QJsonArray svcReq;
        {
            QJsonObject req;
            req.insert(QLatin1String("getServerDateTime"), true);
            req.insert(QLatin1String("getTimeTablePeriod"), false);

            QJsonObject serverInfo;
            serverInfo.insert(QLatin1String("meth"), QLatin1String("ServerInfo"));
            serverInfo.insert(QLatin1String("req"), req);

            svcReq.push_back(serverInfo);
        }

        {
            QJsonObject cfg;
            cfg.insert(QLatin1String("polyEnc"), QLatin1String("GPA"));

            QJsonObject req;
            req.insert(QLatin1String("date"), request.dateTime().toString(QLatin1String("yyyyMMdd")));
            req.insert(QLatin1String("maxJny"), 12);
            req.insert(QLatin1String("stbFltrEquiv"), true);

            QJsonObject stbLoc;
            stbLoc.insert(QLatin1String("extId"), id);
            stbLoc.insert(QLatin1String("state"), QLatin1String("F"));
            stbLoc.insert(QLatin1String("type"), QLatin1String("S"));

            req.insert(QLatin1String("stbLoc"), stbLoc);
            req.insert(QLatin1String("time"), request.dateTime().toString(QLatin1String("hhmmss")));
            req.insert(QLatin1String("type"), QLatin1String("DEP"));

            QJsonObject stationBoard;
            stationBoard.insert(QLatin1String("cfg"), cfg);
            stationBoard.insert(QLatin1String("meth"), QLatin1String("StationBoard"));
            stationBoard.insert(QLatin1String("req"), req);
            svcReq.push_back(stationBoard);
        }

        top.insert(QLatin1String("svcReqL"), svcReq);
    }
    top.insert(QLatin1String("ver"), m_version);

    const auto content = QJsonDocument(top).toJson();
    QUrl url(m_endpoint);
    QUrlQuery query;
    if (!m_micMacSalt.isEmpty()) {
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(content);
        const auto mic = md5.result().toHex();
        query.addQueryItem(QLatin1String("mic"), QString::fromLatin1(mic));

        md5.reset();
        // yes, mic is added as hex-encoded string, and the salt is added as raw bytes
        md5.addData(mic);
        md5.addData(m_micMacSalt);
        query.addQueryItem(QLatin1String("mac"), QString::fromLatin1(md5.result().toHex()));
    }
    if (!m_checksumSalt.isEmpty()) {
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(content);
        md5.addData(m_checksumSalt);
        query.addQueryItem(QLatin1String("checksum"), QString::fromLatin1(md5.result().toHex()));
    }
    url.setQuery(query);

    auto netReq = QNetworkRequest(url);
    netReq.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));

    auto netReply = nam->post(netReq, content);
    qDebug() << netReq.url();
//     qDebug().noquote() << QJsonDocument(top).toJson();
    QObject::connect(netReply, &QNetworkReply::finished, [netReply, reply, this]() {
        qDebug() << netReply->request().url();
        switch (netReply->error()) {
            case QNetworkReply::NoError:
                addResult(reply, m_parser.parseDepartures(netReply->readAll()));
                break;
            default:
                addError(reply, Reply::NetworkError, netReply->errorString());
                qCDebug(Log) << netReply->error() << netReply->errorString();
                break;
        }
        netReply->deleteLater();
    });

    return true;
}

void HafasMgateBackend::setMicMacSalt(const QString &salt)
{
    m_micMacSalt = QByteArray::fromHex(salt.toUtf8());
}

void HafasMgateBackend::setChecksumSalt(const QString &salt)
{
    m_checksumSalt = QByteArray::fromHex(salt.toUtf8());
}

void HafasMgateBackend::setLineModeMap(const QJsonObject& obj)
{
    const auto idx = Line::staticMetaObject.indexOfEnumerator("Mode");
    Q_ASSERT(idx >= 0);
    const auto me = Line::staticMetaObject.enumerator(idx);

    std::unordered_map<int, Line::Mode> modeMap;
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        modeMap[it.key().toInt()] = static_cast<Line::Mode>(me.keyToValue(it.value().toString().toUtf8()));
    }
    m_parser.setLineModeMap(std::move(modeMap));
}
