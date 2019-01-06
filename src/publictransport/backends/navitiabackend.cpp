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

#include "navitiabackend.h"
#include "logging.h"
#include "navitiaparser.h"
#include "cache.h"

#include <KPublicTransport/Departure>
#include <KPublicTransport/DepartureReply>
#include <KPublicTransport/DepartureRequest>
#include <KPublicTransport/Journey>
#include <KPublicTransport/JourneyReply>
#include <KPublicTransport/JourneyRequest>
#include <KPublicTransport/Location>
#include <KPublicTransport/LocationReply>
#include <KPublicTransport/LocationRequest>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

using namespace KPublicTransport;

NavitiaBackend::NavitiaBackend() = default;

bool NavitiaBackend::isSecure() const
{
    return true; // https is hardcoded below
}

bool NavitiaBackend::queryJourney(JourneyReply *reply, QNetworkAccessManager *nam) const
{
    const auto req = reply->request();
    if (!req.from().hasCoordinate() || !req.to().hasCoordinate()) {
        return false;
    }

    QUrl url;
    url.setScheme(QStringLiteral("https"));
    url.setHost(m_endpoint);
    url.setPath(QStringLiteral("/v1") +
        (m_coverage.isEmpty() ? QString() : (QStringLiteral("/coverage/") + m_coverage)) +
        QStringLiteral("/journeys"));

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("from"), QString::number(req.from().longitude()) + QLatin1Char(';') + QString::number(req.from().latitude()));
    query.addQueryItem(QStringLiteral("to"), QString::number(req.to().longitude()) + QLatin1Char(';') + QString::number(req.to().latitude()));
    if (req.dateTime().isValid()) {
        query.addQueryItem(QStringLiteral("datetime"), req.dateTime().toString(QStringLiteral("yyyyMMddThhmmss")));
        query.addQueryItem(QStringLiteral("datetime_represents"), req.dateTimeMode() == JourneyRequest::Arrival ? QStringLiteral("arrival") : QStringLiteral("departure"));
    }

    // TODO: disable reply parts we don't care about
    query.addQueryItem(QStringLiteral("disable_geojson"), QStringLiteral("true")); // ### seems to have no effect?
    query.addQueryItem(QStringLiteral("depth"), QStringLiteral("0")); // ### also has no effect?
    url.setQuery(query);

    QNetworkRequest netReq(url);
    netReq.setRawHeader("Authorization", m_auth.toUtf8());

    qCDebug(Log) << "GET:" << url;
    auto netReply = nam->get(netReq);
    QObject::connect(netReply, &QNetworkReply::finished, [reply, netReply] {
        switch (netReply->error()) {
            case QNetworkReply::NoError:
                addResult(reply, NavitiaParser::parseJourneys(netReply->readAll()));
                break;
            case QNetworkReply::ContentNotFoundError:
                addError(reply, Reply::NotFoundError, NavitiaParser::parseErrorMessage(netReply->readAll()));
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

bool NavitiaBackend::queryDeparture(DepartureReply *reply, QNetworkAccessManager *nam) const
{
    const auto req = reply->request();
    if (req.stop().hasCoordinate()) {
        queryDeparture(reply, req.stop(), nam);
        return true;
    }

    // missing location information to query directly
    LocationRequest locReq;
    locReq.setName(req.stop().name());
    // TODO set max result = 1

    // check if this location query is cached already
    const auto cacheEntry = Cache::lookupLocation(backendId(), locReq.cacheKey());
    switch (cacheEntry.type) {
        case CacheHitType::Negative:
            addError(reply, Reply::NotFoundError, {});
            return false;
        case CacheHitType::Positive:
            if (cacheEntry.data.size() >= 1) {
                queryDeparture(reply, cacheEntry.data[0], nam);
                return true;
            }
            break;
        case CacheHitType::Miss:
            break;
    }

   const auto locReply = postLocationQuery(locReq, nam);
    if (!locReply) {
        return false;
    }
    QObject::connect(locReply, &QNetworkReply::finished, [this, reply, locReply, locReq, nam]() {
        qDebug() << locReply->request().url();
        switch (locReply->error()) {
            case QNetworkReply::NoError:
            {
                const auto res = NavitiaParser::parsePlaces(locReply->readAll());
                Cache::addLocationCacheEntry(backendId(), locReq.cacheKey(), res);
                if (!res.empty()) {
                    queryDeparture(reply, res[0], nam);
                } else {
                    addError(reply, Reply::NotFoundError, QLatin1String("Location query found no results."));
                }
                break;
            }
            case QNetworkReply::ContentNotFoundError:
                addError(reply, Reply::NotFoundError, NavitiaParser::parseErrorMessage(locReply->readAll()));
                Cache::addNegativeLocationCacheEntry(backendId(), locReq.cacheKey());
                break;
            default:
                addError(reply, Reply::NetworkError, locReply->errorString());
                break;
        }
        locReply->deleteLater();
    });

    return true;
}

void NavitiaBackend::queryDeparture(DepartureReply *reply, const Location &loc, QNetworkAccessManager *nam) const
{
    const auto req = reply->request();

    QUrl url;
    url.setScheme(QStringLiteral("https"));
    url.setHost(m_endpoint);
    url.setPath(
        QStringLiteral("/v1/coverage/") +
        (m_coverage.isEmpty() ? QString::number(loc.longitude()) + QLatin1Char(';') + QString::number(loc.latitude()) : m_coverage) +
        QStringLiteral("/coord/") + QString::number(loc.longitude()) + QLatin1Char(';') + QString::number(loc.latitude()) +
        (req.mode() == DepartureRequest::QueryDeparture ? QStringLiteral("/departures") : QStringLiteral("/arrivals"))
    );

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("from_datetime"), req.dateTime().toString(QStringLiteral("yyyyMMddThhmmss")));
    query.addQueryItem(QStringLiteral("disable_geojson"), QStringLiteral("true"));
    query.addQueryItem(QStringLiteral("depth"), QStringLiteral("0"));
    url.setQuery(query);

    QNetworkRequest netReq(url);
    netReq.setRawHeader("Authorization", m_auth.toUtf8());

    qCDebug(Log) << "GET:" << url;
    auto netReply = nam->get(netReq);
    QObject::connect(netReply, &QNetworkReply::finished, [netReply, reply] {
        switch (netReply->error()) {
            case QNetworkReply::NoError:
                addResult(reply, NavitiaParser::parseDepartures(netReply->readAll()));
                break;
            case QNetworkReply::ContentNotFoundError:
                addError(reply, Reply::NotFoundError, NavitiaParser::parseErrorMessage(netReply->readAll()));
                break;
            default:
                addError(reply, Reply::NetworkError, netReply->errorString());
                qCDebug(Log) << netReply->error() << netReply->errorString();
                break;
        }
        netReply->deleteLater();
    });
}

bool NavitiaBackend::queryLocation(LocationReply *reply, QNetworkAccessManager *nam) const
{
    const auto req = reply->request();
    auto netReply = postLocationQuery(req, nam);
    if (!netReply) {
        return false;
    }

    QObject::connect(netReply, &QNetworkReply::finished, [this, netReply, reply] {
        qDebug() << netReply->request().url() << netReply->errorString();
        switch (netReply->error()) {
            case QNetworkReply::NoError:
            {
                std::vector<Location> res;
                if (reply->request().hasCoordinate()) {
                    res = NavitiaParser::parsePlacesNearby(netReply->readAll());
                } else {
                    res = NavitiaParser::parsePlaces(netReply->readAll());
                }
                Cache::addLocationCacheEntry(backendId(), reply->request().cacheKey(), res);
                addResult(reply, std::move(res));
                break;
            }
            case QNetworkReply::ContentNotFoundError:
                addError(reply, Reply::NotFoundError, NavitiaParser::parseErrorMessage(netReply->readAll()));
                Cache::addNegativeLocationCacheEntry(backendId(), reply->request().cacheKey());
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

QNetworkReply* NavitiaBackend::postLocationQuery(const LocationRequest &req, QNetworkAccessManager *nam) const
{
    QUrl url;
    url.setScheme(QStringLiteral("https"));
    url.setHost(m_endpoint);

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("disable_geojson"), QStringLiteral("true"));
    query.addQueryItem(QStringLiteral("depth"), QStringLiteral("0"));
    query.addQueryItem(QStringLiteral("type[]"), QStringLiteral("stop_area"));
    // TODO count

    if (req.hasCoordinate()) {
        url.setPath(
            QStringLiteral("/v1/coord/") + QString::number(req.longitude()) + QLatin1Char(';') + QString::number(req.latitude()) +
            QStringLiteral("/places_nearby")
        );
        // TODO distance
    } else if (!req.name().isEmpty()) {
        url.setPath(QStringLiteral("/v1/places"));
        query.addQueryItem(QStringLiteral("q"), req.name());
    } else {
        return nullptr;
    }

    url.setQuery(query);
    QNetworkRequest netReq(url);
    netReq.setRawHeader("Authorization", m_auth.toUtf8());

    qCDebug(Log) << "GET:" << url;
    return nam->get(netReq);
}
