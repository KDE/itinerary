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

#include <KPublicTransport/DepartureReply>
#include <KPublicTransport/DepartureRequest>
#include <KPublicTransport/Location>

#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

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

    auto netReq = QNetworkRequest(QUrl(m_endpoint));
    netReq.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    auto netReply = nam->post(netReq, QJsonDocument(top).toJson());
    qDebug().noquote() << QJsonDocument(top).toJson();
    QObject::connect(netReply, &QNetworkReply::finished, [netReply, reply]() {
        qDebug() << netReply->errorString();
        qDebug() << netReply->request().url();
        qDebug() << netReply->readAll();
    });

    return false;
}
