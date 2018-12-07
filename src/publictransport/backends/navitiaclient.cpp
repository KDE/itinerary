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

#include "navitiaclient.h"

#include <KPublicTransport/JourneyRequest>
#include <KPublicTransport/Location>

#include <QDateTime>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

using namespace KPublicTransport;

QNetworkReply* NavitiaClient::findJourney(const JourneyRequest &req, QNetworkAccessManager *nam)
{
    QUrl url;
    url.setScheme(QStringLiteral("https"));
    url.setHost(QStringLiteral("api.navitia.io"));
    url.setPath(QStringLiteral("/v1/journeys"));

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("from"), QString::number(req.from().latitude()) + QLatin1Char(';') + QString::number(req.from().longitude()));
    query.addQueryItem(QStringLiteral("to"), QString::number(req.to().latitude()) + QLatin1Char(';') + QString::number(req.to().longitude()));
    if (req.dateTime().isValid()) {
        query.addQueryItem(QStringLiteral("datetime"), req.dateTime().toString(QStringLiteral("yyyyMMddThhmmss")));
        query.addQueryItem(QStringLiteral("datetime_represents"), req.dateTimeMode() == JourneyRequest::Arrival ? QStringLiteral("arrival") : QStringLiteral("departure"));
    }

    // TODO: disable reply parts we don't care about
    query.addQueryItem(QStringLiteral("disable_geojson"), QStringLiteral("true")); // ### seems to have no effect?
    query.addQueryItem(QStringLiteral("depth"), QStringLiteral("0")); // ### also has no effect?
    url.setQuery(query);

    QNetworkRequest netReq(url);
    netReq.setRawHeader("Authorization", "48ed1733-d3f0-445a-9210-9fb36e20a8a3"); // ### this is the test key

    qDebug() << "GET:" << url;
    return nam->get(netReq);
}
