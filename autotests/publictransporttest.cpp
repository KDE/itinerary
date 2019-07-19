/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#include <publictransport.h>

#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/Journey>

#include <QtTest/qtest.h>
#include <QStandardPaths>

using namespace KItinerary;

class PublicTransportTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
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

        TrainReservation res;
        res.setReservationFor(trip);

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

        // old line name parts need to be cleared
        QCOMPARE(newTrip.trainName(), QString());
        QCOMPARE(newTrip.trainNumber(), QLatin1String("R 342"));

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
    }
};

QTEST_GUILESS_MAIN(PublicTransportTest)

#include "publictransporttest.moc"
