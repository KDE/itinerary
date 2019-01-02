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

bool HafasMgateBackend::isSecure() const
{
    return m_endpoint.startsWith(QLatin1String("https"));
}

bool HafasMgateBackend::queryJourney(JourneyReply *reply, QNetworkAccessManager *nam) const
{
    m_parser.setLocationIdentifierType(locationIdentifierType());
    const auto request = reply->request();

    const auto fromId = request.from().identifier(locationIdentifierType());
    if (!fromId.isEmpty()) {
        return queryJourney(reply, fromId, nam);
    }

    LocationRequest locReq;
    locReq.setCoordinate(request.from().latitude(), request.from().longitude());
    locReq.setName(request.from().name());
    // TODO set max result = 1

    // check if this location query is cached already
    const auto cacheEntry = Cache::lookupLocation(backendId(), locReq.cacheKey());
    switch (cacheEntry.type) {
        case CacheHitType::Negative:
            addError(reply, Reply::NotFoundError, {});
            return false;
        case CacheHitType::Positive:
            if (cacheEntry.data.size() >= 1) {
                return queryJourney(reply, cacheEntry.data[0].identifier(locationIdentifierType()), nam);
            }
            break;
        case CacheHitType::Miss:
            break;
    }

    // query from location
    const auto locReply = postLocationQuery(locReq, nam);
    if (!locReply) {
        return false;
    }
    QObject::connect(locReply, &QNetworkReply::finished, [this, reply, locReply, locReq, nam]() {
        switch (locReply->error()) {
            case QNetworkReply::NoError:
            {
                auto res = m_parser.parseLocations(locReply->readAll());
                if (m_parser.error() == Reply::NoError && !res.empty()) {
                    Cache::addLocationCacheEntry(backendId(), locReq.cacheKey(), res);
                    const auto id = res[0].identifier(locationIdentifierType());
                    if (!id.isEmpty()) {
                        queryJourney(reply, id, nam);
                    } else {
                        addError(reply, Reply::NotFoundError, QLatin1String("Location query found no result for departure."));
                    }
                } else {
                    Cache::addNegativeLocationCacheEntry(backendId(), locReq.cacheKey());
                    addError(reply, m_parser.error(), m_parser.errorMessage());
                }
                break;
            }
            default:
                addError(reply, Reply::NetworkError, locReply->errorString());
                qCDebug(Log) << locReply->error() << locReply->errorString();
                break;
        }
        locReply->deleteLater();
    });

    return true;
}

bool HafasMgateBackend::queryJourney(JourneyReply *reply, const QString &fromId, QNetworkAccessManager *nam) const
{
    const auto request = reply->request();

    const auto toId = request.to().identifier(locationIdentifierType());
    if (!toId.isEmpty()) {
        return queryJourney(reply, fromId, toId, nam);
    }

    LocationRequest locReq;
    locReq.setCoordinate(request.to().latitude(), request.to().longitude());
    locReq.setName(request.to().name());
    // TODO set max result = 1

    // check if this location query is cached already
    const auto cacheEntry = Cache::lookupLocation(backendId(), locReq.cacheKey());
    switch (cacheEntry.type) {
        case CacheHitType::Negative:
            addError(reply, Reply::NotFoundError, {});
            return false;
        case CacheHitType::Positive:
            if (cacheEntry.data.size() >= 1) {
                return queryJourney(reply, fromId, cacheEntry.data[0].identifier(locationIdentifierType()), nam);
            }
            break;
        case CacheHitType::Miss:
            break;
    }

    // query to location
    const auto locReply = postLocationQuery(locReq, nam);
    if (!locReply) {
        addError(reply, Reply::NotFoundError, {});
        return false;
    }
    QObject::connect(locReply, &QNetworkReply::finished, [this, fromId, reply, locReply, locReq, nam]() {
        switch (locReply->error()) {
            case QNetworkReply::NoError:
            {
                auto res = m_parser.parseLocations(locReply->readAll());
                if (m_parser.error() == Reply::NoError && !res.empty()) {
                    Cache::addLocationCacheEntry(backendId(), locReq.cacheKey(), res);
                    const auto id = res[0].identifier(locationIdentifierType());
                    if (!id.isEmpty()) {
                        queryJourney(reply, fromId, id, nam);
                    } else {
                        addError(reply, Reply::NotFoundError, QLatin1String("Location query found no result for arrival."));
                    }
                } else {
                    Cache::addNegativeLocationCacheEntry(backendId(), locReq.cacheKey());
                    addError(reply, m_parser.error(), m_parser.errorMessage());
                }
                break;
            }
            default:
                addError(reply, Reply::NetworkError, locReply->errorString());
                qCDebug(Log) << locReply->error() << locReply->errorString();
                break;
        }
        locReply->deleteLater();
    });

    return true;
}

bool HafasMgateBackend::queryJourney(JourneyReply *reply, const QString &fromId, const QString &toId, QNetworkAccessManager *nam) const
{
    qCDebug(Log) << backendId() << fromId << toId;
    const auto request = reply->request();

    QJsonObject tripSearch;
    {
        QJsonObject cfg;
        cfg.insert(QLatin1String("polyEnc"), QLatin1String("GPA"));

        QJsonArray arrLocL;
        QJsonObject arrLoc;
        arrLoc.insert(QLatin1String("extId"), toId);
        arrLoc.insert(QLatin1String("type"), QLatin1String("S")); // 'S' == station
        arrLocL.push_back(arrLoc);

        QJsonArray depLocL;
        QJsonObject depLoc;
        depLoc.insert(QLatin1String("extId"), fromId);
        depLoc.insert(QLatin1String("type"), QLatin1String("S"));
        depLocL.push_back(depLoc);

        QJsonArray jnyFltrL;
        QJsonObject jnyFltr;
        jnyFltr.insert(QLatin1String("mode"), QLatin1String("BIT"));
        jnyFltr.insert(QLatin1String("type"), QLatin1String("PROD"));
        jnyFltr.insert(QLatin1String("value"), QLatin1String("1111111001"));
        jnyFltrL.push_back(jnyFltr);

        QJsonObject req;
        req.insert(QLatin1String("arrLocL"), arrLocL);
        req.insert(QLatin1String("depLocL"), depLocL);
        req.insert(QLatin1String("extChgTime"), -1);
        req.insert(QLatin1String("getEco"), false);
        req.insert(QLatin1String("getIST"), false);
        req.insert(QLatin1String("getPasslist"), true); // ???
        req.insert(QLatin1String("getPolyline"), false);
        req.insert(QLatin1String("jnyFltrL"), jnyFltrL);

        req.insert(QLatin1String("outDate"), request.dateTime().date().toString(QLatin1String("yyyyMMdd")));
        req.insert(QLatin1String("outTime"), request.dateTime().time().toString(QLatin1String("hhmmss")));
        req.insert(QLatin1String("outFrwd"), true);

        tripSearch.insert(QLatin1String("cfg"), cfg);
        tripSearch.insert(QLatin1String("meth"), QLatin1String("TripSearch"));
        tripSearch.insert(QLatin1String("req"), req);
    }

    auto netReply = postRequest(tripSearch, nam);
    QObject::connect(netReply, &QNetworkReply::finished, [netReply, reply, this]() {
        switch (netReply->error()) {
            case QNetworkReply::NoError:
            {
                auto res = m_parser.parseJourneys(netReply->readAll());
                if (m_parser.error() == Reply::NoError) {
                    addResult(reply, std::move(res));
                } else {
                    addError(reply, m_parser.error(), m_parser.errorMessage());
                }
                break;
            }
            default:
                addError(reply, Reply::NetworkError, netReply->errorString());
                qCDebug(Log) << netReply->error() << netReply->errorString();
                break;
        }
        netReply->deleteLater();
    });

    return true;
}

bool HafasMgateBackend::queryDeparture(DepartureReply *reply, QNetworkAccessManager *nam) const
{
    m_parser.setLocationIdentifierType(locationIdentifierType());
    const auto request = reply->request();

    const auto id = request.stop().identifier(locationIdentifierType());
    if (!id.isEmpty()) {
        queryDeparture(reply, id, nam);
        return true;
    }

    // missing the station id
    LocationRequest locReq;
    locReq.setCoordinate(request.stop().latitude(), request.stop().longitude());
    locReq.setName(request.stop().name());
    // TODO set max result = 1

    // check if this location query is cached already
    const auto cacheEntry = Cache::lookupLocation(backendId(), locReq.cacheKey());
    switch (cacheEntry.type) {
        case CacheHitType::Negative:
            addError(reply, Reply::NotFoundError, {});
            return false;
        case CacheHitType::Positive:
            if (cacheEntry.data.size() >= 1) {
                queryDeparture(reply, cacheEntry.data[0].identifier(locationIdentifierType()), nam);
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
                auto res = m_parser.parseLocations(locReply->readAll());
                if (m_parser.error() == Reply::NoError && !res.empty()) {
                    Cache::addLocationCacheEntry(backendId(), locReq.cacheKey(), res);
                    const auto id = res[0].identifier(locationIdentifierType());
                    if (!id.isEmpty()) {
                        queryDeparture(reply, id, nam);
                    } else {
                        addError(reply, Reply::NotFoundError, QLatin1String("Location query found no results."));
                    }
                } else {
                    Cache::addNegativeLocationCacheEntry(backendId(), locReq.cacheKey());
                    addError(reply, m_parser.error(), m_parser.errorMessage());
                }
                break;
            }
            default:
                addError(reply, Reply::NetworkError, locReply->errorString());
                qCDebug(Log) << locReply->error() << locReply->errorString();
                break;
        }
        locReply->deleteLater();
    });

    return true;
}

void HafasMgateBackend::queryDeparture(DepartureReply *reply, const QString &locationId, QNetworkAccessManager *nam) const
{
    const auto request = reply->request();

    QJsonObject stationBoard;
    {
        QJsonObject cfg;
        cfg.insert(QLatin1String("polyEnc"), QLatin1String("GPA"));

        QJsonObject req;
        req.insert(QLatin1String("date"), request.dateTime().toString(QLatin1String("yyyyMMdd")));
        req.insert(QLatin1String("maxJny"), 12);
        req.insert(QLatin1String("stbFltrEquiv"), true);

        QJsonObject stbLoc;
        stbLoc.insert(QLatin1String("extId"), locationId);
        stbLoc.insert(QLatin1String("state"), QLatin1String("F"));
        stbLoc.insert(QLatin1String("type"), QLatin1String("S"));

        req.insert(QLatin1String("stbLoc"), stbLoc);
        req.insert(QLatin1String("time"), request.dateTime().toString(QLatin1String("hhmmss")));
        req.insert(QLatin1String("type"), request.mode() == DepartureRequest::QueryDeparture ? QLatin1String("DEP") : QLatin1String("ARR"));

        stationBoard.insert(QLatin1String("cfg"), cfg);
        stationBoard.insert(QLatin1String("meth"), QLatin1String("StationBoard"));
        stationBoard.insert(QLatin1String("req"), req);
    }

    auto netReply = postRequest(stationBoard, nam);
    QObject::connect(netReply, &QNetworkReply::finished, [netReply, reply, this]() {
        qDebug() << netReply->request().url();
        switch (netReply->error()) {
            case QNetworkReply::NoError:
            {
                auto result = m_parser.parseDepartures(netReply->readAll());
                if (m_parser.error() != Reply::NoError) {
                    addError(reply, m_parser.error(), m_parser.errorMessage());
                    qCDebug(Log) << m_parser.error() << m_parser.errorMessage();
                } else {
                    addResult(reply, std::move(result));
                }
                break;
            }
            default:
                addError(reply, Reply::NetworkError, netReply->errorString());
                qCDebug(Log) << netReply->error() << netReply->errorString();
                break;
        }
        netReply->deleteLater();
    });
}

bool HafasMgateBackend::queryLocation(LocationReply *reply, QNetworkAccessManager *nam) const
{
    m_parser.setLocationIdentifierType(locationIdentifierType());

    const auto req = reply->request();
    const auto netReply = postLocationQuery(req, nam);
    if (!netReply) {
        return false;
    }

    QObject::connect(netReply, &QNetworkReply::finished, [netReply, reply, this]() {
        qDebug() << netReply->request().url();
        switch (netReply->error()) {
            case QNetworkReply::NoError:
            {
                auto res = m_parser.parseLocations(netReply->readAll());
                if (m_parser.error() == Reply::NoError) {
                    Cache::addLocationCacheEntry(backendId(), reply->request().cacheKey(), res);
                    addResult(reply, std::move(res));
                } else {
                    Cache::addNegativeLocationCacheEntry(backendId(), reply->request().cacheKey());
                    addError(reply, m_parser.error(), m_parser.errorMessage());
                }
                break;
            }
            default:
                addError(reply, Reply::NetworkError, netReply->errorString());
                qCDebug(Log) << netReply->error() << netReply->errorString();
                break;
        }
        netReply->deleteLater();
    });

    return true;
}

QNetworkReply* HafasMgateBackend::postRequest(const QJsonObject &svcReq, QNetworkAccessManager *nam) const
{
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
        QJsonArray svcReqs;
        {
            QJsonObject req;
            req.insert(QLatin1String("getServerDateTime"), true);
            req.insert(QLatin1String("getTimeTablePeriod"), false);

            QJsonObject serverInfo;
            serverInfo.insert(QLatin1String("meth"), QLatin1String("ServerInfo"));
            serverInfo.insert(QLatin1String("req"), req);

            svcReqs.push_back(serverInfo);
        }
        svcReqs.push_back(svcReq);
        top.insert(QLatin1String("svcReqL"), svcReqs);
    }
    top.insert(QLatin1String("ver"), m_version);

    const auto content = QJsonDocument(top).toJson(QJsonDocument::Compact);
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
    qCDebug(Log) << netReq.url();
    //qCDebug(Log).noquote() << QJsonDocument(top).toJson();
    return nam->post(netReq, content);
}

QNetworkReply* HafasMgateBackend::postLocationQuery(const LocationRequest &req, QNetworkAccessManager *nam) const
{
    QJsonObject methodObj;
    if (req.hasCoordinate()) {
        QJsonObject cfg;
        cfg.insert(QLatin1String("polyEnc"), QLatin1String("GPA"));

        QJsonObject coord;
        coord.insert(QLatin1String("x"), (int)(req.longitude() * 1000000));
        coord.insert(QLatin1String("y"), (int)(req.latitude() * 1000000));
        QJsonObject ring;
        ring.insert(QLatin1String("cCrd"), coord);
        ring.insert(QLatin1String("maxDist"), 20000); // not sure which unit...

        QJsonObject reqObj;
        reqObj.insert(QLatin1String("ring"), ring);
        // ### make this configurable in LocationRequest
        reqObj.insert(QLatin1String("getStops"), true);
        reqObj.insert(QLatin1String("getPOIs"), false);
        reqObj.insert(QLatin1String("maxLoc"), 12);

        methodObj.insert(QLatin1String("cfg"), cfg);
        methodObj.insert(QLatin1String("meth"), QLatin1String("LocGeoPos"));
        methodObj.insert(QLatin1String("req"), reqObj);

    } else if (!req.name().isEmpty()) {
        QJsonObject cfg;
        cfg.insert(QLatin1String("polyEnc"), QLatin1String("GPA"));

        QJsonObject loc;
        loc.insert(QLatin1String("name"), req.name()); // + '?' for auto completion search?
        loc.insert(QLatin1String("type"), QLatin1String("S")); // station: S, address: A, POI: P

        QJsonObject input;
        input.insert(QLatin1String("field"), QLatin1String("S"));
        input.insert(QLatin1String("loc"), loc);
        // ### make this configurable in LocationRequest
        input.insert(QLatin1String("maxLoc"), 12);

        QJsonObject reqObj;
        reqObj.insert(QLatin1String("input"), input);

        methodObj.insert(QLatin1String("cfg"), cfg);
        methodObj.insert(QLatin1String("meth"), QLatin1String("LocMatch"));
        methodObj.insert(QLatin1String("req"), reqObj);

    } else {
        return nullptr;
    }

    return postRequest(methodObj, nam);
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

QString HafasMgateBackend::locationIdentifierType() const
{
    return m_locationIdentifierType.isEmpty() ? backendId() : m_locationIdentifierType;
}
