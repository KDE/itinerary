// SPDX-FileCopyrightText: 2025 Jonah Brüchert <jbb@kaidan.im>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "reservationonlinepostprocessor.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Datatypes>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/LocationUtil>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/Line>

#include <KCountry>

#include <QCoroNetwork>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkInformation>
#include <QNetworkRequest>
#include <QObject>
#include <QUrlQuery>

#include <chrono>
#include <ranges>

#include "applicationcontroller.h"
#include "logging.h"
#include "reservationmanager.h"
#include "settings.h"

namespace ranges = std::ranges;

using namespace Qt::StringLiterals;

ReservationOnlinePostprocessor::ReservationOnlinePostprocessor(
    ReservationManager *reservationMgr,
    Settings *settings,
    std::function<QNetworkAccessManager *()> namFactory)
    : QObject()
    , m_resMgr(reservationMgr)
    , m_settings(settings)
    , m_namFactory(std::move(namFactory))
{
    if (QNetworkInformation::loadDefaultBackend()) {
        connect(QNetworkInformation::instance(),
                &QNetworkInformation::reachabilityChanged,
                this,
                [this](auto reachability) {
                    if (reachability == QNetworkInformation::Reachability::Online) {
                        for (const QString &batchId : m_resMgr->batches()) {
                            handleBatchChange(batchId);
                        }
                    }
                });
    } else {
        qCWarning(Log) << "Failed to load any QNetworkInformation backend. Will not be able to "
                          "resolve reservation places when connectivity is restored";
    }
    connect(reservationMgr,
            &ReservationManager::batchAdded,
            this,
            &ReservationOnlinePostprocessor::handleBatchChange);
    connect(reservationMgr,
            &ReservationManager::batchContentChanged,
            this,
            &ReservationOnlinePostprocessor::handleBatchChange);
}

QCoro::Task<> ReservationOnlinePostprocessor::handleBatchChange(QString batchId)
{
    if (!m_settings->queryLiveData()) {
        co_return;
    }

    auto reservation = m_resMgr->reservation(batchId);
    auto updatedReservation = co_await processReservation(reservation);

    if (!updatedReservation) {
        co_return;
    }

    // Don't react to our own changes
    disconnect(m_resMgr, &ReservationManager::batchContentChanged, this, nullptr);
    m_resMgr->updateReservation(batchId, *updatedReservation);

    connect(m_resMgr,
            &ReservationManager::batchContentChanged,
            this,
            &ReservationOnlinePostprocessor::handleBatchChange);
}

QCoro::Task<std::optional<QVariant>> ReservationOnlinePostprocessor::processReservation(
    QVariant reservation)
{
    const auto locationsRealistic = [](const KItinerary::Place &departurePlace,
                                       const QDateTime &departureTime,
                                       const KItinerary::Place &arrivalPlace,
                                       const QDateTime &arrivalTime,
                                       KPublicTransport::Line::Mode mode) -> bool {
        if (!departurePlace.geo().isValid() || !arrivalPlace.geo().isValid()) {
            return false;
        }

        using namespace KPublicTransport;

        // Check if the result is realistic.
        auto duration = (arrivalTime - departureTime);
        auto distance_kilometers = KItinerary::LocationUtil::distance(departurePlace.geo(),
                                                                      arrivalPlace.geo())
                                   / 1000.0;
        auto hours = float(std::chrono::duration_cast<std::chrono::hours>(duration).count());

        auto speed = distance_kilometers / hours;

        // The speed limit has to be fairly conservative as most vehicles don't move on a straight line.
        float speedLimit = [=]() -> float {
            switch (mode) {
            case Line::Train:
            case Line::LongDistanceTrain:
                return 260.0;
            case Line::Bus:
            case Line::Coach:
                return 120.0;
            default:
                return 200.0;
            }
        }();

        if (speed > speedLimit) {
            qCWarning(Log) << "Ignored coordinates from nominatim for" << departurePlace.name()
                           << "and" << arrivalPlace.name()
                           << "because they were unrealistic. Speed:" << speed << "km/h";
            return false;
        }

        return true;
    };

    if (KItinerary::JsonLd::isA<KItinerary::TrainReservation>(reservation)) {
        auto trainReservation = KItinerary::JsonLd::convert<KItinerary::TrainReservation>(
            reservation);
        auto trip = trainReservation.reservationFor().value<KItinerary::TrainTrip>();

        const auto changedDepartureStation = co_await processTrainStation(trip.departureStation());
        const auto changedArrivalStation = co_await processTrainStation(trip.arrivalStation());

        if (changedDepartureStation || changedArrivalStation) {
            if (locationsRealistic(changedDepartureStation.value_or(trip.departureStation()),
                                   trip.departureTime(),
                                   changedArrivalStation.value_or(trip.arrivalStation()),
                                   trip.arrivalTime(),
                                   KPublicTransport::Line::Train)) {
                if (changedDepartureStation) {
                    trip.setDepartureStation(*changedDepartureStation);
                }
                if (changedArrivalStation) {
                    trip.setArrivalStation(*changedArrivalStation);
                }

                trainReservation.setReservationFor(trip);
                co_return trainReservation;
            }
        }
    } else if (KItinerary::JsonLd::isA<KItinerary::BusReservation>(reservation)) {
        auto busReservation = KItinerary::JsonLd::convert<KItinerary::BusReservation>(reservation);
        auto trip = busReservation.reservationFor().value<KItinerary::BusTrip>();

        const auto changedDepartureStation = co_await processBusStation(trip.departureBusStop());
        const auto changedArrivalStation = co_await processBusStation(trip.arrivalBusStop());

        if (changedDepartureStation || changedArrivalStation) {
            if (locationsRealistic(changedDepartureStation.value_or(trip.departureBusStop()),
                                   trip.departureTime(),
                                   changedArrivalStation.value_or(trip.arrivalBusStop()),
                                   trip.arrivalTime(),
                                   KPublicTransport::Line::Train)) {
                if (changedDepartureStation) {
                    trip.setDepartureBusStop(*changedDepartureStation);
                }
                if (changedArrivalStation) {
                    trip.setArrivalBusStop(*changedArrivalStation);
                }

                busReservation.setReservationFor(trip);
                co_return busReservation;
            }
        }
    }

    co_return {};
}

/** Checks for fallback place names based on location identifiers we might
 *  get from some barcode imports.
 *  Those cannot be queried via Nominatim for example.
 */
[[nodiscard]] static bool hasIdentifierName(const KItinerary::Place &place)
{
    const auto idx = place.identifier().indexOf(':'_L1);
    return idx > 0 && place.identifier().endsWith(place.name()) && (idx + 1 + place.name().size()) == place.identifier().size();
}

QCoro::Task<std::optional<KItinerary::TrainStation>>
ReservationOnlinePostprocessor::processTrainStation(KItinerary::TrainStation station)
{
    if (station.geo().isValid() || hasIdentifierName(station)) {
        co_return {};
    }

    const std::vector allowedTags = {"railway:station"_L1, "railway:halt"_L1};

    const auto results = co_await queryNominatim(station, u"railway station"_s);
    bool changed = applyResult(results, station, allowedTags);

    co_return changed ? std::optional{station} : std::nullopt;
}

QCoro::Task<std::optional<KItinerary::BusStation>> ReservationOnlinePostprocessor::processBusStation(
    KItinerary::BusStation station)
{
    if (station.geo().isValid() || hasIdentifierName(station)) {
        co_return {};
    }

    const std::vector allowedTags = {"amenity:bus_station"_L1};

    const auto results = co_await queryNominatim(station, u"bus station"_s);
    bool changed = applyResult(results, station, allowedTags);

    co_return changed ? std::optional{station} : std::nullopt;
}

QCoro::Task<QJsonArray> ReservationOnlinePostprocessor::queryNominatim(
    const KItinerary::Place &place, const QString &amenityType)
{
    const QString amenityQuery = amenityType % u" " % place.name();

    QUrl url = QUrl(u"https://nominatim.openstreetmap.org/search"_s);
    QUrlQuery query;
    query.addQueryItem(u"amenity"_s, amenityQuery);
    query.addQueryItem(u"layer"_s, u"poi,railway"_s);
    query.addQueryItem(u"extratags"_s, u"1"_s);
    query.addQueryItem(u"format"_s, u"jsonv2"_s);

    if (!place.address().addressCountry().isEmpty()) {
        query.addQueryItem(u"countrycodes"_s, place.address().addressCountry());
    }

    if (!place.address().isEmpty()) {
        const QString countryCode = place.address().addressCountry();
        const QString countryName = KCountry::fromAlpha2(countryCode).name();

        const auto addQueryParam = [&](const QString &param, const QString &value) {
            if (value.isEmpty()) {
                return;
            }
            query.addQueryItem(param, value);
        };

        addQueryParam(u"street"_s, place.address().streetAddress());
        addQueryParam(u"city"_s, place.address().addressLocality());
        addQueryParam(u"state"_s, place.address().addressRegion());
        addQueryParam(u"country"_s, countryName);
        addQueryParam(u"postalcode"_s, place.address().postalCode());
    }
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader,
                  ApplicationController::userAgent());

    auto reply = co_await m_namFactory()->get(req);
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qCWarning(Log) << "Error while resolving station using Nominatim:" << reply->errorString();
        co_return {};
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    co_return doc.array();
}

bool ReservationOnlinePostprocessor::applyResult(const QJsonArray &results,
                                                 KItinerary::Place &place,
                                                 const std::vector<QLatin1String> &allowedTags)
{
    for (const auto result : results) {
        const auto object = result.toObject();
        const QString tag = object["category"_L1].toString() % u":" % object["type"_L1].toString();
        if (ranges::find(allowedTags, tag) == allowedTags.end()) {
            continue;
        }

        KItinerary::GeoCoordinates coords;
        coords.setLatitude(object["lat"_L1].toString().toDouble());
        coords.setLongitude(object["lon"_L1].toString().toDouble());
        place.setGeo(coords);

        const auto extratags = object["extratags"_L1].toObject();
        if (extratags.contains("uic_ref"_L1)) {
            const QString uic_ref = extratags["uic_ref"_L1].toString();

            // Germany is currently mis-tagged: https://invent.kde.org/pim/itinerary/-/merge_requests/446#note_1331724
            if (!uic_ref.startsWith(u"80")) {
                place.setIdentifier(u"uic:" % uic_ref);
            }
        }
        if (extratags.contains("ref:ibnr"_L1)) {
            place.setIdentifier(u"ibnr:" % extratags["ref:ibnr"_L1].toString());
        }

        return true;
    }
    return false;
}
