/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "publictransport.h"

#include <KItinerary/Reservation>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/Journey>
#include <KPublicTransport/JourneyRequest>
#include <KPublicTransport/Manager>

#include <QStandardPaths>
#include <QTimeZone>
#include <QtTest/qtest.h>

using namespace KItinerary;

class PublicTransportTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "Europe/Zurich");
        QStandardPaths::setTestModeEnabled(true);
    }

    void testApplyJourneySection()
    {
        TrainStation dep;
        dep.setName(QStringLiteral("Visp"));
        dep.setGeo(GeoCoordinates(46.2939, 7.8814));
        TrainStation arr;
        arr.setName(QStringLiteral("Randa"));
        arr.setGeo(GeoCoordinates(46.0998, 7.7814));
        TrainTrip trip;
        trip.setDepartureStation(dep);
        trip.setDepartureTime(QDateTime({2017, 9, 10}, {14, 8}));
        trip.setDeparturePlatform(QStringLiteral("3"));
        trip.setArrivalStation(arr);
        trip.setArrivalTime(QDateTime({2017, 9, 10}, {14, 53}));
        trip.setArrivalPlatform(QStringLiteral("1"));
        trip.setTrainName(QStringLiteral("R"));
        trip.setTrainNumber(QStringLiteral("241"));

        Seat seat;
        seat.setSeatNumber(QStringLiteral("42"));
        seat.setSeatingType(QStringLiteral("2nd Class"));
        Ticket ticket;
        ticket.setTicketToken(QStringLiteral("qr:TICKETTOKEN"));
        ticket.setTicketedSeat(seat);

        TrainReservation res;
        res.setReservationFor(trip);
        res.setReservedTicket(ticket);

        KPublicTransport::Location from, to;
        from.setName(QStringLiteral("Visp"));
        to.setName(QStringLiteral("Zermatt"));
        KPublicTransport::Route route;
        KPublicTransport::Line line;
        line.setName(QStringLiteral("R 342"));
        route.setLine(line);
        KPublicTransport::JourneySection section;
        section.setFrom(from);
        section.setScheduledDepartureTime(QDateTime({2017, 9, 10}, {15, 8}));
        section.setTo(to);
        section.setScheduledArrivalTime(QDateTime({2017, 9, 10}, {16, 15}));
        section.setRoute(route);
        section.setScheduledArrivalPlatform(QStringLiteral("2"));

        const auto newRes = PublicTransport::applyJourneySection(res, section).value<TrainReservation>();
        const auto newTrip = newRes.reservationFor().value<TrainTrip>();

        // times need to be updated
        QCOMPARE(newTrip.departureTime(), section.scheduledDepartureTime());
        QCOMPARE(newTrip.arrivalTime(), section.scheduledArrivalTime());

        // times have correct timezones now (while input was in local time)
        QCOMPARE(newTrip.departureTime().timeSpec(), Qt::TimeZone);
        QCOMPARE(newTrip.departureTime().timeZone().id(), "Europe/Zurich");

        // old line name parts need to be cleared
        QCOMPARE(newTrip.trainName(), QString());
        QCOMPARE(newTrip.trainNumber(), QLatin1StringView("R 342"));

        // old platform information need to be cleared
        QCOMPARE(newTrip.departurePlatform(), QString());
        QCOMPARE(newTrip.arrivalPlatform(), section.scheduledArrivalPlatform());

        // departure location didn't change, so retaining details is ok
        const auto newDep = newTrip.departureStation();
        QCOMPARE(newDep.name(), section.from().name());
        QVERIFY(newDep.geo().isValid());

        // arrival location did change, so old data must not remain
        const auto newArr = newTrip.arrivalStation();
        QCOMPARE(newArr.name(), section.to().name());
        QVERIFY(!newArr.geo().isValid());

        // locations got country data assigned
        QCOMPARE(newDep.address().addressCountry(), QLatin1StringView("CH"));

        // ticket token is preserved, but seat reservation is cleared
        const auto newTicket = newRes.reservedTicket().value<Ticket>();
        QCOMPARE(newTicket.ticketToken(), ticket.ticketToken());
        QCOMPARE(newTicket.ticketedSeat().seatNumber(), QString());
        QCOMPARE(newTicket.ticketedSeat().seatingType(), seat.seatingType()); // class is preserved
    }

    void testWarn()
    {
        KPublicTransport::JourneySection section;
        QVERIFY(!PublicTransport().warnAboutSection(section));
        section.setMode(KPublicTransport::JourneySection::Walking);
        QVERIFY(!PublicTransport().warnAboutSection(section));
        section.setDistance(2000);
        QVERIFY(PublicTransport().warnAboutSection(section));
    }

    void testBackendSelection()
    {
        Organization provider;
        provider.setIdentifier(QStringLiteral("uic:1181"));
        TrainTrip trip;
        trip.setProvider(provider);
        TrainReservation res;
        res.setReservationFor(trip);

        KPublicTransport::Manager mgr;
        KPublicTransport::JourneyRequest req;

        mgr.setBackendEnabled(QLatin1StringView("at_oebb"), true);
        PublicTransport::selectBackends(req, &mgr, res);
        QCOMPARE(req.backendIds(), QStringList({QLatin1StringView("at_oebb")}));
        mgr.setBackendEnabled(QLatin1StringView("at_oebb"), false);
        PublicTransport::selectBackends(req, &mgr, res);
        QCOMPARE(req.backendIds(), QStringList());

        provider.setIdentifier(QStringLiteral("uic:80"));
        trip.setProvider(provider);
        res.setReservationFor(trip);
        mgr.setBackendEnabled(QLatin1StringView("de_db"), true);
        PublicTransport::selectBackends(req, &mgr, res);
        QCOMPARE(req.backendIds(), QStringList({QLatin1StringView("de_db")}));
    }
};

QTEST_GUILESS_MAIN(PublicTransportTest)

#include "publictransporttest.moc"
