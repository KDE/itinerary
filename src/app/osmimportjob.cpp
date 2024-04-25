/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "osmimportjob.h"

#include "applicationcontroller.h"

#include <KItinerary/Place>
#include <KItinerary/Reservation>

#include <KOSM/Element>
#include <KOSM/IO>

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>

using namespace Qt::Literals::StringLiterals;

OsmImportJob::OsmImportJob(OSM::Type type, OSM::Id id, QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent)
    , m_type(type)
    , m_id(id)
{
    QUrl url;
    url.setScheme("https"_L1);
    url.setHost("www.openstreetmap.org"_L1);
    if (type == OSM::Type::Node) {
        url.setPath("/api/0.6/"_L1 + QLatin1StringView(OSM::typeName(type)) + '/'_L1 + QString::number(id));
    } else {
        url.setPath("/api/0.6/"_L1 + QLatin1StringView(OSM::typeName(type)) + '/'_L1 + QString::number(id) + "/full"_L1);
    }

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, ApplicationController::userAgent());
    auto reply = nam->get(req);
    reply->setParent(this);
    connect(reply, &QNetworkReply::finished, this, [reply, this]() { handleReply(reply); });
}

OsmImportJob::~OsmImportJob() = default;

QVariant OsmImportJob::result() const
{
    return m_result;
}

QString OsmImportJob::errorMessage() const
{
    return m_errorMsg;
}

void OsmImportJob::handleReply(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        m_errorMsg = reply->errorString();
    } else {
        OSM::DataSet dataSet;
        const auto reader = OSM::IO::readerForMimeType(u"application/vnd.openstreetmap.data+xml", &dataSet);
        reader->read(reply);
        if (reader->hasError()) {
            m_errorMsg = reader->errorString();
        } else {
            OSM::for_each(dataSet, [&dataSet](OSM::Element e) { e.recomputeBoundingBox(dataSet); },  OSM::IncludeRelations | OSM::IncludeWays);
            m_result = convertElement(OSM::lookupElement(dataSet, m_type, m_id));
        }
    }
    Q_EMIT finished();
}

[[nodiscard]] static KItinerary::PostalAddress addressFromOsm(OSM::Element e)
{
    KItinerary::PostalAddress addr;
    addr.setAddressCountry(QString::fromUtf8(e.tagValue("addr:country", "contact:country")));
    addr.setAddressRegion(QString::fromUtf8(e.tagValue("addr:state")));
    addr.setAddressLocality(QString::fromUtf8(e.tagValue("addr:city", "contact:city")));
    addr.setPostalCode(QString::fromUtf8(e.tagValue("addr:postcode", "contact:postcode")));
    addr.setStreetAddress(QString::fromUtf8(e.tagValue("addr:street", "contact:street", "addr:housename")));
    if (const auto n = e.tagValue("addr:housenumber", "contact:housenumber"); !n.isEmpty()) {
        addr.setStreetAddress(addr.streetAddress() + ' '_L1 + QString::fromUtf8(n));
    }
    return addr;
}

template <typename P>
static void convertContactData(P &p, OSM::Element e)
{
    p.setUrl(QUrl(QString::fromUtf8(e.tagValue("website", "contact:website", "url", "operator:website"))));
    p.setTelephone(QString::fromUtf8(e.tagValue("contact:phone", "phone", "telephone", "operator:phone")));
    p.setEmail(QString::fromUtf8(e.tagValue("contact:email", "email", "operator:email")));
}

QVariant OsmImportJob::convertElement(OSM::Element e)
{
    const auto tourism = e.tagValue("tourism");
    if (tourism == "apartment" || tourism == "chalet" || tourism == "guest_house" || tourism == "hostel" || tourism == "hotel") {
        KItinerary::LodgingBusiness h;
        h.setName(QString::fromUtf8(e.tagValue(OSM::Languages::fromQLocale(QLocale()), "name", "loc_name", "int_name", "brand")));
        h.setGeo(KItinerary::GeoCoordinates(e.center().latF(), e.center().lonF())); // TODO for ways we could check for the entrance node even
        h.setAddress(addressFromOsm(e));
        convertContactData(h, e);
        return h;
    }

    const auto amenity = e.tagValue("amenity");
    if (amenity == "bar" || amenity == "biergarten" || amenity == "cafe" || amenity == "fast_food"
     || amenity == "ice_cream" || amenity == "pub" || amenity == "restaurant")
    {
        KItinerary::FoodEstablishment r;
        r.setName(QString::fromUtf8(e.tagValue(OSM::Languages::fromQLocale(QLocale()), "name", "loc_name", "int_name", "brand")));
        r.setGeo(KItinerary::GeoCoordinates(e.center().latF(), e.center().lonF())); // TODO for ways we could check for the entrance node even
        r.setAddress(addressFromOsm(e));
        convertContactData(r, e);
        return r;
    }

    return {};
}

#include "moc_osmimportjob.cpp"
