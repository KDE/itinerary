/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "publictransport.h"
#include "reservationhelper.h"
#include "logging.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>
#include <KItinerary/Ticket>

#include <KPublicTransport/Attribution>
#include <KPublicTransport/Backend>
#include <KPublicTransport/Journey>
#include <KPublicTransport/RentalVehicle>
#include <KPublicTransport/Stopover>
#include <KPublicTransport/StopoverRequest>

#include <QDateTime>
#include <QDebug>
#include <QUrl>

static bool isTrainMode(KPublicTransport::Line::Mode mode)
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

static bool isBusMode(KPublicTransport::Line::Mode mode)
{
    using namespace KPublicTransport;
    switch (mode) {
        case Line::Bus:
        case Line::BusRapidTransit:
        case Line::Coach:
            return true;
        default:
            return false;
    }
}

KPublicTransport::Location PublicTransport::locationFromPlace(const QVariant& place, const QVariant &reservation)
{
    using namespace KItinerary;

    KPublicTransport::Location loc;

    if (JsonLd::isA<FlightReservation>(reservation) || JsonLd::isA<TrainReservation>(reservation) || JsonLd::isA<BusReservation>(reservation)) {
        loc.setName(KItinerary::LocationUtil::name(place));
    } else {
        const auto addr = KItinerary::LocationUtil::address(place);
        loc.setStreetAddress(addr.streetAddress());
        loc.setPostalCode(addr.postalCode());
        loc.setLocality(addr.addressLocality());
        loc.setRegion(addr.addressRegion());
        loc.setCountry(addr.addressCountry());
    }

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

QString PublicTransport::lineIcon(const QVariant &line) const
{
    using namespace KPublicTransport;
    const auto l = line.value<Line>();
    const auto logo = l.logo();
    if (!logo.isEmpty()) {
        return logo;
    }
    const auto modeLogo = l.modeLogo();
    if (!modeLogo.isEmpty()) {
        return modeLogo;
    }
    return lineModeIcon(l.mode());
}

QString PublicTransport::lineModeIcon(int lineMode) const
{
    using namespace KPublicTransport;
    switch (lineMode) {
        case Line::Air:
            return QStringLiteral("qrc:///images/flight.svg");
        case Line::Boat:
        case Line::Ferry:
            return QStringLiteral("qrc:///images/ferry.svg");
        case Line::Bus:
            return QStringLiteral("qrc:///images/bus.svg");
        case Line::BusRapidTransit:
        case Line::Coach:
            return QStringLiteral("qrc:///images/coach.svg");
        case Line::Funicular:
            return QStringLiteral("qrc:///images/Funicular.svg");
        case Line::LocalTrain:
        case Line::Train:
            return QStringLiteral("qrc:///images/train.svg");
        case Line::LongDistanceTrain:
            return QStringLiteral("qrc:///images/longdistancetrain.svg");
        case Line::Metro:
            return QStringLiteral("qrc:///images/subway.svg");
        case Line::RailShuttle:
        case Line::RapidTransit:
            return QStringLiteral("qrc:///images/rapidtransit.svg");
        case Line::Shuttle:
            return QStringLiteral("qrc:///images/shuttle.svg");
        case Line::Taxi:
            return QStringLiteral("qrc:///images/taxi.svg");
        case Line::Tramway:
            return QStringLiteral("qrc:///images/tramway.svg");
        case Line::RideShare:
            return QStringLiteral("qrc:///images/car.svg");
    }

    return QStringLiteral("question");
}

QString PublicTransport::rentalVehicleIcon(const KPublicTransport::RentalVehicle &vehicle) const
{
    using namespace KPublicTransport;
    switch (vehicle.type()) {
        case RentalVehicle::Unknown:
            return {};
        case RentalVehicle::Bicycle:
        case RentalVehicle::Pedelec: // TODO
            return QStringLiteral("qrc:///images/bike.svg");
        case RentalVehicle::ElectricMoped:
        case RentalVehicle::ElectricKickScooter:
            return QStringLiteral("question"); // TODO
        case RentalVehicle::Car:
            return QStringLiteral("qrc:///images/car.svg");
    }
    return QStringLiteral("question");
}

static QString individualTransportIcon(const KPublicTransport::IndividualTransport &it)
{
    using namespace KPublicTransport;

    switch (it.mode()) {
        case IndividualTransport::Bike:
             return QStringLiteral("qrc:///images/bike.svg");
        case IndividualTransport::Car:
            return QStringLiteral("qrc:///images/car.svg");
        case IndividualTransport::Walk:
            return QStringLiteral("qrc:///images/walk.svg");
    }

    return QStringLiteral("question");
}

QString PublicTransport::journeySectionIcon(const KPublicTransport::JourneySection &journeySection) const
{
    using namespace KPublicTransport;
    switch (journeySection.mode()) {
        case JourneySection::Invalid:
            break;
        case JourneySection::PublicTransport:
            return lineIcon(journeySection.route().line());
        case JourneySection::Walking:
            return QStringLiteral("qrc:///images/walk.svg");
        case JourneySection::Waiting:
            return QStringLiteral("qrc:///images/wait.svg");
        case JourneySection::Transfer:
            return QStringLiteral("qrc:///images/transfer.svg");
        case JourneySection::RentedVehicle:
            return rentalVehicleIcon(journeySection.rentalVehicle());
        case JourneySection::IndividualTransport:
            return individualTransportIcon(journeySection.individualTransport());
    }

    return QStringLiteral("question");
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

QVariant PublicTransport::reservationFromJourneySection(const KPublicTransport::JourneySection &section)
{
    using namespace KItinerary;
    if (isTrainMode(section.route().line().mode())) {
        return ::applyJourneySection(TrainReservation(), section);
    }
    if (isBusMode(section.route().line().mode())) {
        return ::applyJourneySection(BusReservation(), section);
    }

    qCWarning(Log) << "Unsupported section type:" << section.route().line().mode();
    return {};
}

QVariant PublicTransport::applyJourneySection(const QVariant &res, const KPublicTransport::JourneySection &section)
{
    using namespace KItinerary;

    if (JsonLd::isA<TrainReservation>(res)) {
        return ::applyJourneySection(res.value<TrainReservation>(), section);
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return ::applyJourneySection(res.value<BusReservation>(), section);
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

bool PublicTransport::isSameMode(const QVariant &res, KPublicTransport::Line::Mode mode)
{
    using namespace KPublicTransport;

    if (KItinerary::JsonLd::isA<KItinerary::TrainReservation>(res)) {
        return isTrainMode(mode);
    } else if (KItinerary::JsonLd::isA<KItinerary::BusReservation>(res)) {
        return isBusMode(mode);
    } else if (KItinerary::JsonLd::isA<KItinerary::FlightReservation>(res)) {
        return mode == Line::Air;
    } else if (res.isValid()) {
        qCWarning(Log) << "unexpected reservation type!?" << res;
    }

    return false;
}

bool PublicTransport::isSameMode(const QVariant &res, const KPublicTransport::JourneySection &section)
{
    return isSameMode(res, section.route().line().mode());
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
        l.push_back(QLatin1String("<a href=\"") + attr.url().toString() + QLatin1String("\">") + attr.name() + QLatin1String("</a>"));
    }
    return l.join(QLatin1String(", "));
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
    auto id = loc.identifier(QLatin1String("ibnr"));
    if (!id.isEmpty()) {
        return QLatin1String("ibnr:") + id;
    }
    id = loc.identifier(QLatin1String("uic"));
    if (!id.isEmpty()) {
        return QLatin1String("uic:") + id;
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

#include "moc_publictransport.cpp"
