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

#include "hafasquerybackend.h"
#include "logging.h"

#include <KPublicTransport/Departure>
#include <KPublicTransport/DepartureReply>
#include <KPublicTransport/DepartureRequest>
#include <KPublicTransport/Location>

#include <QDateTime>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

using namespace KPublicTransport;

HafasQueryBackend::HafasQueryBackend() = default;

bool HafasQueryBackend::isSecure() const
{
    return m_endpoint.startsWith(QLatin1String("https://"));
}

bool HafasQueryBackend::queryDeparture(DepartureReply *reply, QNetworkAccessManager *nam) const
{
    const auto request = reply->request();
    const auto stationId = request.stop().identifier(QLatin1String("ibnr")); // only seems to exist with IBNR so far? if that turns out to be wrong, do the same as in the mgate backend
    if (stationId.isEmpty()) {
        // TODO support location queries like in the mgate variant
        return false;
    }

    QUrl url(m_endpoint);
    url.setPath(url.path() + QLatin1String("/stboard.exe/en")); // dn/nn?

    QUrlQuery query;
    query.addQueryItem(QLatin1String("boardType"), request.mode() == DepartureRequest::QueryDeparture ? QLatin1String("dep") : QLatin1String("arr"));
    query.addQueryItem(QLatin1String("disableEquivs"), QLatin1String("0"));
    query.addQueryItem(QLatin1String("maxJourneys"), QLatin1String("12"));
    query.addQueryItem(QLatin1String("input"), stationId);
    query.addQueryItem(QLatin1String("date"), request.dateTime().date().toString(QLatin1String("dd.MM.yy")));
    query.addQueryItem(QLatin1String("time"), request.dateTime().time().toString(QLatin1String("hh:mm")));
    query.addQueryItem(QLatin1String("L"), QLatin1String("vs_java3"));
    query.addQueryItem(QLatin1String("start"), QLatin1String("yes"));
    url.setQuery(query);
    qDebug() << url;

    auto netReply = nam->get(QNetworkRequest(url));
    QObject::connect(netReply, &QNetworkReply::finished, [this, netReply, reply]() {
        qDebug() << netReply->request().url();
        if (netReply->error() != QNetworkReply::NoError) {
            addError(reply, Reply::NetworkError, netReply->errorString());
            qCDebug(Log) << reply->error() << reply->errorString();
            return;
        }
        auto res = m_parser.parseStationBoardResponse(netReply->readAll(), reply->request().mode() == DepartureRequest::QueryArrival);
        // TODO error handling
        addResult(reply, std::move(res));
        netReply->deleteLater();
    });

    return true;
}
