/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "publictransport.h"
#include "reservationhelper.h"
#include "logging.h"

#include <KItinerary/BoatTrip>
#include <KItinerary/BusTrip>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>
#include <KItinerary/Ticket>

#include <KPublicTransport/Attribution>
#include <KPublicTransport/Backend>
#include <KPublicTransport/Journey>
#include <KPublicTransport/RentalVehicle>
#include <KPublicTransport/Stopover>
#include <KPublicTransport/StopoverRequest>

#include <KLocalizedString>

#include <QDateTime>
#include <QDebug>
#include <QUrl>
#include <kpublictransport/journey.h>

bool PublicTransport::isTrainMode(KPublicTransport::Line::Mode mode)
{
    using namespace KPublicTransport;
    switch (mode) {
        case Line::Train:
        case Line::Funicular:
        case Line::LocalTrain:
        case Line::LongDistanceTrain:
        case Line::Metro:
        case Line::RailShuttle:
        case Line::RapidTransit:
        case Line::Tramway:
            return true;
        default:
            return false;
    }
}

bool PublicTransport::isBusMode(KPublicTransport::Line::Mode mode)
{
    using namespace KPublicTransport;
    switch (mode) {
        case Line::Bus:
        case Line::BusRapidTransit:
        case Line::Coach:
        case Line::Shuttle:
            return true;
        default:
            return false;
    }
}

bool PublicTransport::isBoatMode(KPublicTransport::Line::Mode mode)
{
    using namespace KPublicTransport;
    switch (mode) {
        case Line::Boat:
        case Line::Ferry:
            return true;
        default:
            return false;
    }
}

KPublicTransport::Location PublicTransport::locationFromPlace(const QVariant& place, const QVariant &reservation)
{
    using namespace KItinerary;

    KPublicTransport::Location loc;

    if (JsonLd::isA<FlightReservation>(reservation) || JsonLd::isA<TrainReservation>(reservation)
     || JsonLd::isA<BusReservation>(reservation) || JsonLd::isA<BoatReservation>(reservation)) {
        loc.setName(KItinerary::LocationUtil::name(place));
    }
    if (JsonLd::isA<TrainReservation>(reservation) || JsonLd::isA<BusReservation>(reservation)) {
        loc.setType(KPublicTransport::Location::Stop);
    }
    const auto addr = KItinerary::LocationUtil::address(place);
    loc.setStreetAddress(addr.streetAddress());
    loc.setPostalCode(addr.postalCode());
    loc.setLocality(addr.addressLocality());
    loc.setRegion(addr.addressRegion());
    loc.setCountry(addr.addressCountry());

    const auto geo = KItinerary::LocationUtil::geo(place);
    loc.setCoordinate(geo.latitude(), geo.longitude());

    if (KItinerary::JsonLd::canConvert<KItinerary::Place>(place)) {
        const auto p = KItinerary::JsonLd::convert<KItinerary::Place>(place);
        if (!p.identifier().isEmpty()) {
            const auto idSplit = p.identifier().split(QLatin1Char(':'));
            if (idSplit.size() == 2) {
                loc.setIdentifier(idSplit.at(0), idSplit.at(1));
            }
        }
    }

    return loc;
}

QVariant PublicTransport::trainStationFromLocation(const KPublicTransport::Location &loc) const
{
    return PublicTransport::placeFromLocation<KItinerary::TrainStation>(loc);
}

QVariant PublicTransport::busStationFromLocation(const KPublicTransport::Location &loc) const
{
    return PublicTransport::placeFromLocation<KItinerary::BusStation>(loc);
}

QString PublicTransport::journeySectionLabel(const KPublicTransport::JourneySection &journeySection) const
{
    using namespace KPublicTransport;
    switch (journeySection.mode()) {
        case JourneySection::Invalid:
            break;
        case JourneySection::PublicTransport:
            return journeySection.route().line().name();
        case JourneySection::Walking:
            return i18n("Walk");
        case JourneySection::Waiting:
            return i18n("Wait");
        case JourneySection::Transfer:
            break; // ?
        case JourneySection::RentedVehicle:
            switch (journeySection.rentalVehicle().type()) {
                case RentalVehicle::Unknown:
                    break;
                case RentalVehicle::Bicycle:
                case RentalVehicle::Pedelec:
                    return i18n("Rented bicycle");
                case RentalVehicle::ElectricMoped:
                    return i18n("Rented moped");
                case RentalVehicle::ElectricKickScooter:
                    return i18n("Rented kick scooter");
                case RentalVehicle::Car:
                    return i18n("Rented car");
            }
            break;
        case JourneySection::IndividualTransport:
            switch (journeySection.individualTransport().mode()) {
                case IndividualTransport::Bike:
                    return i18n("Bicycle");
                case IndividualTransport::Car:
                    return i18n("Car");
                case IndividualTransport::Walk:
                    return i18n("Walk");
            }
            break;
    }
    return {};
}

static KItinerary::Ticket clearSeat(KItinerary::Ticket ticket)
{
    auto seat = ticket.ticketedSeat();
    seat.setSeatNumber(QString());
    seat.setSeatRow(QString());
    seat.setSeatSection(QString());
    ticket.setTicketedSeat(seat);
    return ticket;
}

static KItinerary::TrainReservation applyJourneySection(KItinerary::TrainReservation res, const KPublicTransport::JourneySection &section)
{
    using namespace KItinerary;

    auto trip = res.reservationFor().value<TrainTrip>();
    trip.setDepartureTime(section.scheduledDepartureTime());
    trip.setArrivalTime(section.scheduledArrivalTime());
    trip.setTrainNumber(section.route().line().name());
    trip.setTrainName(section.route().line().modeString());
    trip.setDeparturePlatform(section.scheduledDeparturePlatform());
    trip.setArrivalPlatform(section.scheduledArrivalPlatform());

    trip.setDepartureStation(PublicTransport::updateToLocation(trip.departureStation(), section.from()));
    trip.setArrivalStation(PublicTransport::updateToLocation(trip.arrivalStation(), section.to()));

    res.setReservationFor(trip);
    res.setReservedTicket(clearSeat(res.reservedTicket().value<Ticket>()));
    return res;
}

static KItinerary::BusReservation applyJourneySection(KItinerary::BusReservation res, const KPublicTransport::JourneySection &section)
{
    using namespace KItinerary;

    auto trip = res.reservationFor().value<BusTrip>();
    trip.setDepartureTime(section.scheduledDepartureTime());
    trip.setArrivalTime(section.scheduledArrivalTime());
    trip.setBusNumber(section.route().line().name());
    trip.setBusName(section.route().line().modeString());
    trip.setDeparturePlatform(section.scheduledDeparturePlatform());
    trip.setArrivalPlatform(section.scheduledArrivalPlatform());

    trip.setDepartureBusStop(PublicTransport::updateToLocation(trip.departureBusStop(), section.from()));
    trip.setArrivalBusStop(PublicTransport::updateToLocation(trip.arrivalBusStop(), section.to()));

    res.setReservationFor(trip);
    res.setReservedTicket(clearSeat(res.reservedTicket().value<Ticket>()));
    return res;
}

static KItinerary::BoatReservation applyJourneySection(KItinerary::BoatReservation res, const KPublicTransport::JourneySection &section)
{
    using namespace KItinerary;

    auto trip = res.reservationFor().value<BoatTrip>();
    trip.setDepartureTime(section.scheduledDepartureTime());
    trip.setArrivalTime(section.scheduledArrivalTime());
    // TODO ferry line name
    trip.setDepartureBoatTerminal(PublicTransport::updateToLocation(trip.departureBoatTerminal(), section.from()));
    trip.setArrivalBoatTerminal(PublicTransport::updateToLocation(trip.arrivalBoatTerminal(), section.to()));

    res.setReservationFor(trip);
    return res;
}

static QVariant postProcessOne(const QVariant &res)
{
    KItinerary::ExtractorPostprocessor postProc;
    postProc.process({res});
    const auto result = postProc.result();
    if (result.size() == 1) {
        return result.at(0);
    }
    return res;
}

QVariant PublicTransport::reservationFromJourneySection(const KPublicTransport::JourneySection &section)
{
    using namespace KItinerary;
    if (isTrainMode(section.route().line().mode())) {
        return postProcessOne(::applyJourneySection(TrainReservation(), section));
    }
    if (isBusMode(section.route().line().mode())) {
        return postProcessOne(::applyJourneySection(BusReservation(), section));
    }
    if (isBoatMode(section.route().line().mode())) {
        return postProcessOne(::applyJourneySection(BoatReservation(), section));
    }

    qCWarning(Log) << "Unsupported section type:" << section.route().line().mode();
    return {};
}

QVariant PublicTransport::applyJourneySection(const QVariant &res, const KPublicTransport::JourneySection &section)
{
    using namespace KItinerary;

    if (JsonLd::isA<TrainReservation>(res)) {
        return postProcessOne(::applyJourneySection(res.value<TrainReservation>(), section));
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return postProcessOne(::applyJourneySection(res.value<BusReservation>(), section));
    }
    if (JsonLd::isA<BoatReservation>(res)) {
        return postProcessOne(::applyJourneySection(res.value<BoatReservation>(), section));
    }

    qCWarning(Log) << res.typeName() << "Unsupported section type!";
    return res;
}

QVariant PublicTransport::mergeDeparture(const QVariant &res, const KPublicTransport::Stopover &dep)
{
    using namespace KItinerary;

    QVariant newRes;
    if (isTrainMode(dep.route().line().mode())) {
        TrainTrip trip;
        trip.setDepartureStation(placeFromLocation<TrainStation>(dep.stopPoint()));
        trip.setDepartureTime(dep.scheduledDepartureTime());
        trip.setDeparturePlatform(dep.scheduledPlatform());
        TrainReservation trainRes;
        trainRes.setReservationFor(trip);
        newRes = trainRes;
    } else if (isBusMode(dep.route().line().mode())) {
        BusTrip trip;
        trip.setDepartureBusStop(placeFromLocation<BusStation>(dep.stopPoint()));
        trip.setDepartureTime(dep.scheduledDepartureTime());
        trip.setDeparturePlatform(dep.scheduledPlatform());
        BusReservation busRes;
        busRes.setReservationFor(trip);
        newRes = busRes;
    }

    return MergeUtil::merge(newRes, res);
}

QVariant PublicTransport::mergeArrival(const QVariant &res, const KPublicTransport::Stopover &arr)
{
    using namespace KItinerary;

    QVariant newRes;
    if (isTrainMode(arr.route().line().mode())) {
        TrainTrip trip;
        trip.setArrivalStation(placeFromLocation<TrainStation>(arr.stopPoint()));
        trip.setArrivalTime(arr.scheduledArrivalTime());
        trip.setArrivalPlatform(arr.scheduledPlatform());
        TrainReservation trainRes;
        trainRes.setReservationFor(trip);
        newRes = trainRes;
    } else if (isBusMode(arr.route().line().mode())) {
        BusTrip trip;
        trip.setArrivalBusStop(placeFromLocation<BusStation>(arr.stopPoint()));
        trip.setArrivalTime(arr.scheduledArrivalTime());
        trip.setArrivalPlatform(arr.scheduledPlatform());
        BusReservation busRes;
        busRes.setReservationFor(trip);
        newRes = busRes;
    }

    return MergeUtil::merge(newRes, res);
}

QVariant PublicTransport::mergeJourney(const QVariant &res, const KPublicTransport::JourneySection &journey)
{
    auto newRes = mergeDeparture(res, journey.departure());
    newRes = mergeArrival(newRes, journey.arrival());
    return newRes;
}

KPublicTransport::StopoverRequest PublicTransport::stopoverRequestForPlace(const QVariant &place, const QDateTime &dt) const
{
    KPublicTransport::StopoverRequest req;
    req.setDateTime(std::max(dt, QDateTime::currentDateTime()));
    req.setStop(PublicTransport::locationFromPlace(place, {}));
    req.setDownloadAssets(true);
    return req;
}

QString PublicTransport::attributionSummary(const QVariantList& attributions) const
{
    QStringList l;
    l.reserve(attributions.size());
    for (const auto &v : attributions) {
        const auto attr = v.value<KPublicTransport::Attribution>();
        const auto multi = std::count_if(attributions.begin(), attributions.end(), [attr](const auto &other) {
            return other.template value<KPublicTransport::Attribution>().name() == attr.name();
        });
        if (multi > 1) {
            l.push_back(QLatin1StringView("<a href=\"") + attr.url().toString() + QLatin1StringView("\">") + attr.name() + QLatin1StringView(" (") + attr.license() + QLatin1StringView(")</a>"));
        } else {
            l.push_back(QLatin1StringView("<a href=\"") + attr.url().toString() + QLatin1StringView("\">") + attr.name() + QLatin1StringView("</a>"));
        }
    }
    return QLocale().createSeparatedList(l);
}

bool PublicTransport::warnAboutSection(const KPublicTransport::JourneySection &section) const
{
    using namespace KPublicTransport;

    switch (section.mode()) {
        case JourneySection::Invalid:
            return false;
        case JourneySection::Walking:
        case JourneySection::Transfer:
            return section.duration() > 20*60 || section.distance() > 1000;
        case JourneySection::Waiting:
            return section.duration() > 20*60;
        case JourneySection::PublicTransport:
            return std::any_of(section.loadInformation().begin(), section.loadInformation().end(), [](const auto &load) { return load.load() == Load::Full; });
        case JourneySection::RentedVehicle:
            switch (section.rentalVehicle().type()) {
                case RentalVehicle::Unknown:
                    return false;
                case RentalVehicle::Bicycle:
                    return section.duration() > 60*60 || section.distance() > 20000;
                case RentalVehicle::ElectricKickScooter:
                case RentalVehicle::ElectricMoped:
                case RentalVehicle::Pedelec:
                case RentalVehicle::Car:
                    return false; // ???
            }
            break;
        case JourneySection::IndividualTransport:
            switch (section.individualTransport().mode()) {
                case IndividualTransport::Walk:
                    return section.duration() > 20*60 || section.distance() > 1000;
                case IndividualTransport::Bike:
                    return section.duration() > 60*60 || section.distance() > 20000;
                case IndividualTransport::Car:
                    return false;
            }
            break;
    }

    return false;
}

QString PublicTransport::idenfitierFromLocation(const KPublicTransport::Location &loc)
{
    auto id = loc.identifier(QLatin1StringView("ibnr"));
    if (!id.isEmpty()) {
        return QLatin1StringView("ibnr:") + id;
    }
    id = loc.identifier(QLatin1StringView("uic"));
    if (!id.isEmpty()) {
        return QLatin1StringView("uic:") + id;
    }
    return {};
}

KItinerary::PostalAddress PublicTransport::addressFromLocation(const KPublicTransport::Location &loc)
{
    KItinerary::PostalAddress addr;
    addr.setStreetAddress(loc.streetAddress());
    addr.setPostalCode(loc.postalCode());
    addr.setAddressLocality(loc.locality());
    addr.setAddressRegion(loc.region());
    addr.setAddressCountry(loc.country());
    return addr;
}

KPublicTransport::JourneySection PublicTransport::firstTransportSection(const KPublicTransport::Journey &journey)
{
    for (const auto &section : journey.sections()) {
        if (section.mode() == KPublicTransport::JourneySection::PublicTransport) {
            return section;
        }
    }

    return {};
}

KPublicTransport::JourneySection PublicTransport::lastTransportSection(const KPublicTransport::Journey &journey)
{
    for (auto it = journey.sections().rbegin(); it != journey.sections().rend(); ++it) {
        if ((*it).mode() == KPublicTransport::JourneySection::PublicTransport) {
            return (*it);
        }
    }

    return {};
}

QStringList PublicTransport::suitableBackendsForReservation(KPublicTransport::Manager *mgr, const QVariant &res)
{
    using namespace KPublicTransport;
    QStringList backendIds;

    const auto companyCode = ReservationHelper::uicCompanyCode(res);
    if (companyCode.size() == 4) {
        for (const auto &backend : mgr->backends()) {
            if (!mgr->isBackendEnabled(backend.identifier())) {
                continue;
            }
            for (const auto cov : { CoverageArea::Realtime, CoverageArea::Regular, CoverageArea::Any }) {
                if (backend.coverageArea(cov).uicCompanyCodes().contains(companyCode)) {
                    backendIds.push_back(backend.identifier());
                    break;
                }
            }
        }
    }

    const auto vdvOrgId = ReservationHelper::vdvOrganizationId(res);
    if (!vdvOrgId.isEmpty()) {
        for (const auto &backend : mgr->backends()) {
            if (!mgr->isBackendEnabled(backend.identifier())) {
                continue;
            }
            for (const auto cov : { CoverageArea::Realtime, CoverageArea::Regular, CoverageArea::Any }) {
                if (backend.coverageArea(cov).vdvOrganizationIds().contains(vdvOrgId)) {
                    backendIds.push_back(backend.identifier());
                    break;
                }
            }
        }
    }

    return backendIds;
}

KOSMIndoorMap::Platform::Mode PublicTransport::lineModeToPlatformMode(KPublicTransport::Line::Mode lineMode)
{
    using namespace KPublicTransport;

    switch (lineMode) {
        case Line::Unknown:
        case Line::AerialLift:
        case Line::Air:
        case Line::Boat:
        case Line::Ferry:
        case Line::Taxi:
        case Line::RideShare:
            return KOSMIndoorMap::Platform::Unknown;
        case Line::Bus:
        case Line::BusRapidTransit:
        case Line::Coach:
        case Line::Shuttle:
            return KOSMIndoorMap::Platform::Bus;
        case Line::Funicular:
        case Line::LocalTrain:
        case Line::LongDistanceTrain:
        case Line::RailShuttle:
        case Line::RapidTransit:
        case Line::Train:
            return KOSMIndoorMap::Platform::Rail;
        case Line::Metro:
            return KOSMIndoorMap::Platform::Subway;
        case Line::Tramway:
            return KOSMIndoorMap::Platform::Tram;
    }

    return KOSMIndoorMap::Platform::Unknown;
}

KPublicTransport::Location PublicTransport::copyLocation(const KPublicTransport::Location &loc)
{
    return loc;
}

bool PublicTransport::isSameStopoverForLayout(const KPublicTransport::Stopover &lhs, const KPublicTransport::Stopover &rhs)
{
    if (!KPublicTransport::Stopover::isSame(lhs, rhs)) {
        return false;
    }

    const auto lhsPlatform = lhs.hasExpectedPlatform() ? lhs.expectedPlatform() : lhs.scheduledPlatform();
    const auto rhsPlatform = rhs.hasExpectedPlatform() ? rhs.expectedPlatform() : rhs.scheduledPlatform();
    return lhsPlatform == rhsPlatform;
}

QList<QGeoCoordinate>PublicTransport::pathToGeoCoordinates(const KPublicTransport::JourneySection &jny)
{
    QList<QGeoCoordinate> result;

    const auto path = jny.path();
    if (path.isEmpty()) {
        result.push_back({jny.departure().stopPoint().latitude(), jny.departure().stopPoint().longitude()});
        for (const auto &s : jny.intermediateStops()) {
            if (!s.stopPoint().hasCoordinate()) {
                continue;
            }

            result.push_back({s.stopPoint().latitude(), s.stopPoint().longitude()});
        }
        result.push_back({jny.arrival().stopPoint().latitude(), jny.arrival().stopPoint().longitude()});
        return result;
    }

    for (const auto &section : path.sections()) {
        const auto path = section.path();
        for (const auto &p : path) {
            result.push_back({p.y(), p.x()});
        }
    }

    return result;
}

KPublicTransport::Load::Category PublicTransport::maximumOccupancy(const QList<KPublicTransport::LoadInfo> &loadInfo)
{
    using namespace KPublicTransport;
    auto o = Load::Unknown;
    for (const auto &l : loadInfo) {
        o = std::max(o, l.load());
    }
    return o;
}

#include "moc_publictransport.cpp"
