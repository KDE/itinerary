/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "itineraryextractor.h"

#include <KItinerary/Datatypes>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/LocationUtil>

#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/Reservation>

#include <KPkPass/Pass>

#include <KLocalizedString>

#include <QFileInfo>
#include <QJsonArray>

#include <optional>

using namespace KFileMetaData;
using namespace KItinerary;

ItineraryExtractor::ItineraryExtractor(QObject *parent)
    : ExtractorPlugin(parent)
{
}

QStringList ItineraryExtractor::mimetypes() const
{
    return QStringList{QStringLiteral("application/vnd.apple.pkpass")};
}

void KFileMetaData::ItineraryExtractor::extract(ExtractionResult *result)
{
    QFileInfo fi(result->inputUrl());
    if (!fi.exists()) {
        return;
    }

    auto contextDt = fi.birthTime();
    if (!contextDt.isValid()) {
        contextDt = fi.lastModified();
    }

    const QScopedPointer<KPkPass::Pass> pass(KPkPass::Pass::fromFile(result->inputUrl(), nullptr));
    if (pass.isNull()) {
        return;
    }

    result->add(Property::Title, pass->description());
    result->add(Property::Author, pass->organizationName());

    ExtractorEngine engine;
    engine.setContextDate(contextDt);

    ExtractorPostprocessor postproc;
    postproc.setContextDate(contextDt);

    engine.setContent(QVariant::fromValue(pass.data()), result->inputMimetype());

    const auto json = JsonLdDocument::fromJson(engine.extract());
    postproc.process(json);

    std::optional<Reservation> res;

    for (const auto &jsonRes : json) {
        if (JsonLd::isA<Event>(jsonRes)) { // promote Event to EventReservation
            EventReservation ev;
            ev.setReservationFor(jsonRes);
            res = ev;
            break;
        } else if (JsonLd::canConvert<Reservation>(jsonRes)) {
            res = JsonLd::convert<Reservation>(jsonRes);
            break;
        }
    }

    if (!res) {
        return;
    }

    if (const QString reservationNumber = res->reservationNumber(); !reservationNumber.isEmpty()) {
        result->add(Property::Comment, reservationNumber);
    }

    if (JsonLd::isA<Event>(res->reservationFor())) {
        const auto event = res->reservationFor().value<Event>();
        // simplified() as some event names have line breaks and other rubbish in them
        if (const QString eventName = event.name().simplified(); !eventName.isEmpty()) {
            // only if it's not the same as the Title
            if (pass->description() != eventName) {
                result->add(Property::Subject, eventName);
            }
        }

        const QString placeName = KItinerary::LocationUtil::name(event.location());
        if (!placeName.isEmpty()) {
            result->add(Property::Location, placeName);
        }

        const GeoCoordinates geo = KItinerary::LocationUtil::geo(event.location());
        if (geo.isValid()) {
            result->add(Property::PhotoGpsLatitude, geo.latitude());
            result->add(Property::PhotoGpsLongitude, geo.longitude());
        }
    } else if (JsonLd::isA<Flight>(res->reservationFor())) {
        const auto flight = res->reservationFor().value<Flight>();
        result->add(Property::Subject,
                    i18nc("Airline Flightnumber from Airport to Airport on Date",
                          "%1%2 from %3 to %4 on %5",
                          flight.airline().iataCode(),
                          flight.flightNumber(),
                          flight.departureAirport().iataCode(),
                          flight.arrivalAirport().iataCode(),
                          QLocale().toString(flight.departureDay(), QLocale::ShortFormat)));
    }
}

#include "moc_itineraryextractor.cpp"
