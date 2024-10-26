/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "publictransportmatcher.h"

#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <KPublicTransport/Journey>
#include <KPublicTransport/Line>
#include <KPublicTransport/Stopover>

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QTimeZone>
#include <QtTest/qtest.h>

#define s(x) QStringLiteral(x)

class PublicTransportMatcherTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testMatchRoute()
    {
        KPublicTransport::Route route;
        KPublicTransport::Line line;
        line.setName(s("RE 13"));
        route.setLine(line);
        QVERIFY(PublicTransportMatcher::isSameRoute(route, s("RE"), s("13")));
        QVERIFY(PublicTransportMatcher::isSameRoute(route, QString(), s("RE13")));
        QVERIFY(!PublicTransportMatcher::isSameRoute(route, s("RE"), s("20072")));

        route.setName(s("RE 20072"));
        QVERIFY(PublicTransportMatcher::isSameRoute(route, s("RE"), s("13")));
        QVERIFY(PublicTransportMatcher::isSameRoute(route, s("RE"), s("20072")));
        QVERIFY(PublicTransportMatcher::isSameRoute(route, QString(), s("RE20072")));
    }

    void testMatchSubJourney()
    {
        const auto journey = KPublicTransport::Journey::fromJson(
            QJsonDocument::fromJson(Test::readFile(QLatin1StringView(SOURCE_DIR "/data/publictransport/db-wifi-journey.json"))).object());
        QCOMPARE(journey.sections().size(), 1);

        KItinerary::TrainStation departure, arrival;
        departure.setName(QStringLiteral("Berlin Hbf (tief)"));
        departure.setGeo(KItinerary::GeoCoordinates(52.5225, 13.3695));
        arrival.setName(QStringLiteral("Hamm (Westf) Hbf"));
        KItinerary::TrainTrip trip;
        trip.setDepartureStation(departure);
        trip.setDepartureTime(QDateTime({2021, 12, 21}, {7, 46}));
        trip.setArrivalStation(arrival);
        trip.setArrivalTime(QDateTime({2021, 12, 21}, {10, 48}));
        trip.setTrainName(QStringLiteral("ICE 944"));
        KItinerary::TrainReservation res;
        res.setReservationFor(trip);

        auto subJny = PublicTransportMatcher::subJourneyForReservation(res, journey.sections()[0]);
        QCOMPARE(subJny.mode(), KPublicTransport::JourneySection::PublicTransport);
        QCOMPARE(subJny.departure().stopPoint().name(), QLatin1StringView("Berlin Hbf"));
        QCOMPARE(subJny.arrival().stopPoint().name(), QLatin1StringView("Hamm (Westf) Hbf"));

        trip.setDepartureTime(QDateTime({2021, 12, 21}, {7, 46}, QTimeZone("Europe/Berlin")));
        res.setReservationFor(trip);
        subJny = PublicTransportMatcher::subJourneyForReservation(res, journey.sections()[0]);
        QCOMPARE(subJny.mode(), KPublicTransport::JourneySection::PublicTransport);

        trip.setTrainName(QStringLiteral("ICE 954"));
        res.setReservationFor(trip);
        subJny = PublicTransportMatcher::subJourneyForReservation(res, journey.sections()[0]);
        QCOMPARE(subJny.mode(), KPublicTransport::JourneySection::Invalid);

        trip.setTrainName(QStringLiteral("ICE 944"));
        trip.setDepartureTime(QDateTime({2021, 12, 21}, {8, 2}));
        res.setReservationFor(trip);
        subJny = PublicTransportMatcher::subJourneyForReservation(res, journey.sections()[0]);
        QCOMPARE(subJny.mode(), KPublicTransport::JourneySection::Invalid);

        arrival.setName(QStringLiteral("Dortmund Hbf"));
        trip.setDepartureTime(QDateTime({2021, 12, 21}, {7, 46}));
        trip.setArrivalStation(arrival);
        res.setReservationFor(trip);
        subJny = PublicTransportMatcher::subJourneyForReservation(res, journey.sections()[0]);
        QCOMPARE(subJny.mode(), KPublicTransport::JourneySection::Invalid);
    }
};

QTEST_GUILESS_MAIN(PublicTransportMatcherTest)

#include "publictransportmatchertest.moc"
