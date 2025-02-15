/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "nominatimimportjob.h"

#include "applicationcontroller.h"

#include <KItinerary/Event>
#include <KItinerary/Place>
#include <KItinerary/Reservation>

#include <KOSM/Element>
#include <KOSM/IO>

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>

#include <initializer_list>

using namespace Qt::Literals;

NominatimImportJob::NominatimImportJob(OSM::Type type, OSM::Id id, QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
    , m_type(type)
    , m_id(id)
{
    QUrl url;
    url.setScheme("https"_L1);
    url.setHost("nominatim.openstreetmap.org"_L1);
    url.setPath("/lookup"_L1);
    QUrlQuery query;
    switch (type) {
        case OSM::Type::Null:
            assert(false);
        case OSM::Type::Node:
            query.addQueryItem(u"osm_ids"_s, 'N'_L1 + QString::number(id));
            break;
        case OSM::Type::Way:
            query.addQueryItem(u"osm_ids"_s, 'W'_L1 + QString::number(id));
            break;
        case OSM::Type::Relation:
            query.addQueryItem(u"osm_ids"_s, 'R'_L1 + QString::number(id));
            break;
    }
    query.addQueryItem(u"extratags"_s, u"1"_s);
    query.addQueryItem(u"format"_s, u"geocodejson"_s);
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, ApplicationController::userAgent());
    auto reply = nam->get(req);
    reply->setParent(this);
    connect(reply, &QNetworkReply::finished, this, [reply, this]() {
        handleReply(reply);
    });
}

NominatimImportJob::~NominatimImportJob() = default;

QVariant NominatimImportJob::result() const
{
    return m_result;
}

QString NominatimImportJob::errorMessage() const
{
    return m_errorMsg;
}

void NominatimImportJob::handleReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        m_errorMsg = reply->errorString();
    } else {
        const auto featureCollection = QJsonDocument::fromJson(reply->readAll()).object();
        const auto features = featureCollection.value("features"_L1).toArray();
        for (const auto &feature : features) {
            m_result = convertElement(feature.toObject());
        }

    }
    Q_EMIT finished();
}

[[nodiscard]] static KItinerary::PostalAddress addressFromNominatim(const QJsonObject &obj)
{
    KItinerary::PostalAddress addr;
    addr.setAddressCountry(obj.value("country_code"_L1).toString().toUpper());
    addr.setAddressRegion(obj.value("state"_L1).toString());
    addr.setAddressLocality(obj.value("city"_L1).toString());
    addr.setPostalCode(obj.value("postcode"_L1).toString());
    addr.setStreetAddress(obj.value("street"_L1).toString());
    if (const auto n = obj.value("housenumber"_L1).toString(); !n.isEmpty()) {
        addr.setStreetAddress(addr.streetAddress() + ' '_L1 + n);
    }
    return addr;
}

[[nodiscard]] static KItinerary::GeoCoordinates geoFromNominatim(const QJsonObject &obj)
{
    const auto coords = obj.value("coordinates"_L1).toArray();
    if (obj.value("type"_L1).toString() != "Point"_L1 || coords.size() != 2) {
        return {};
    }
    return KItinerary::GeoCoordinates(coords.at(1).toDouble(), coords.at(0).toDouble());
}

template <typename T>
[[nodiscard]] static QString multiValue(const QJsonObject &obj, std::initializer_list<T> keys)
{
    for (const auto &key : keys) {
        const auto val = obj.value(QLatin1StringView(key));
        if (val.isString()) {
            return val.toString();
        }
    }
    return {};
}

template<typename P>
static void convertContactData(P &p, const QJsonObject &obj)
{
    p.setUrl(QUrl(multiValue(obj, {"website", "contact:website", "url", "operator:website"})));
    p.setTelephone(multiValue(obj, {"contact:phone", "phone", "telephone", "operator:phone"}));
    p.setEmail(multiValue(obj, {"contact:email", "email", "operator:email"}));
}

QVariant NominatimImportJob::convertElement(const QJsonObject &obj)
{
    const auto properties = obj.value("properties"_L1).toObject();
    const auto geocoding = properties.value("geocoding"_L1).toObject();
    const auto extra = geocoding.value("extra"_L1).toObject();
    const auto geometry = obj.value("geometry"_L1).toObject();
    const auto key = geocoding.value("osm_key"_L1).toString();
    const auto value = geocoding.value("osm_value"_L1).toString();
    if ((key == "tourism"_L1 && (value == "apartment"_L1 || value == "chalet"_L1 || value == "guest_house"_L1 || value == "hostel"_L1 || value == "hotel"_L1))
     || (key == "building"_L1 && value == "hotel"_L1)) {
        KItinerary::LodgingBusiness h;
        h.setName(geocoding.value("name"_L1).toString());
        h.setGeo(geoFromNominatim(geometry));
        h.setAddress(addressFromNominatim(geocoding));
        convertContactData(h, extra);
        return h;
    }

    if (key == "amenity"_L1 && (value == "bar"_L1 || value == "biergarten"_L1 || value == "cafe"_L1 || value == "fast_food"_L1 || value == "ice_cream"_L1 || value == "pub"_L1 || value == "restaurant"_L1)) {
        KItinerary::FoodEstablishment r;
        r.setName(geocoding.value("name"_L1).toString());
        r.setGeo(geoFromNominatim(geometry));
        r.setAddress(addressFromNominatim(geocoding));
        convertContactData(r, extra);
        return r;
    }

    if ((key == "amenity"_L1 && (value == "cinema"_L1 || value == "conference_centre"_L1 || value == "events_venue"_L1 || value == "exhibition_centre"_L1 || value == "theatre"_L1))
     || (key == "leisure"_L1 && (value == "escape_game"_L1 || value == "minature_golf"_L1 || value == "stadium"_L1 || value == "water_park"_L1))
     || (key == "office"_L1 && !value.isEmpty())
     || (key == "building"_L1 && !value.isEmpty() && value != "no"_L1)
     || (key == "tourism"_L1 && (value == "attraction"_L1 || value == "gallery"_L1 || value == "museum"_L1 || value == "theme_park"_L1 || value == "zoo"_L1))) {
        KItinerary::Place loc;
        loc.setName(geocoding.value("name"_L1).toString());
        loc.setGeo(geoFromNominatim(geometry));
        loc.setAddress(addressFromNominatim(geocoding));
        loc.setTelephone(multiValue(extra, {"contact:phone", "phone", "telephone", "operator:phone"}));
        KItinerary::Event ev;
        ev.setLocation(loc);
        ev.setUrl(QUrl(multiValue(extra, {"website", "contact:website", "url", "operator:website"})));
        return ev;
    }

    return {};
}

#include "moc_nominatimimportjob.cpp"
