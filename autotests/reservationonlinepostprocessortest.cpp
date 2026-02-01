/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mocknetworkaccessmanager.h"
#include "testhelper.h"

#include "importcontroller.h"
#include "reservationmanager.h"
#include "reservationonlinepostprocessor.h"
#include "settings.h"

#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <QSignalSpy>
#include <QStandardPaths>
#include <QUrl>
#include <QtTest/qtest.h>

using namespace Qt::Literals;

class ReservationOnlinePostprocessorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testNominatimPostProc()
    {
        m_nam.requests.clear();
        m_nam.replies.push({QNetworkReply::NoError, 200, "[{\"place_id\":174132780,\"licence\":\"Data © OpenStreetMap contributors, ODbL 1.0. http://osm.org/copyright\",\"osm_type\":\"node\",\"osm_id\":1839787470,\"lat\":\"48.1849883\",\"lon\":\"16.3779391\",\"category\":\"railway\",\"type\":\"station\",\"place_rank\":30,\"importance\":0.4673787380320214,\"addresstype\":\"railway\",\"name\":\"Vienna Central Station\",\"display_name\":\"Vienna Central Station, 1, Am Hauptbahnhof, Quartier Belvedere, KG Favoriten, Favoriten, Vienna, 1100, Austria\",\"extratags\":{\"level\": \"1\", \"train\": \"yes\", \"network\": \"VOR\", \"uic_ref\": \"8101003\", \"operator\": \"ÖBB-Infrastruktur AG\", \"ref:ibnr\": \"8103000\", \"uic_name\": \"Wien Hbf\", \"wikidata\": \"Q697300\", \"platforms\": \"10\", \"wikipedia\": \"de:Wien Hauptbahnhof\", \"start_date\": \"2012-12-09\", \"wheelchair\": \"yes\", \"railway:ref\": \"Wbf\", \"railway:ref:DB\": \"XAWIE\", \"internet_access\": \"wlan\", \"network:wikidata\": \"Q2516485\", \"public_transport\": \"station\", \"railway:position\": \"0.000 / 99.947\", \"toilets:wheelchair\": \"yes\", \"internet_access:fee\": \"no\", \"internet_access:ssid\": \"OEBB-station\", \"railway:position:exact\": \"0.000 / 99.947\"},\"boundingbox\":[\"48.1799883\",\"48.1899883\",\"16.3729391\",\"16.3829391\"]}]", QString()});
        m_nam.replies.push({QNetworkReply::NoError, 200, "[{\"place_id\":67518675,\"licence\":\"Data © OpenStreetMap contributors, ODbL 1.0. http://osm.org/copyright\",\"osm_type\":\"node\",\"osm_id\":337863237,\"lat\":\"47.2632966\",\"lon\":\"11.4010207\",\"category\":\"railway\",\"type\":\"station\",\"place_rank\":30,\"importance\":0.42528692828871034,\"addresstype\":\"railway\",\"name\":\"Innsbruck Hauptbahnhof\",\"display_name\":\"Innsbruck Hauptbahnhof, Autoverladung, Innsbruck, Tyrol, 6020, Austria\",\"extratags\":{\"ele\": \"582\", \"train\": \"yes\", \"network\": \"Verkehrsverbund Tirol\", \"uic_ref\": \"8101187\", \"operator\": \"ÖBB-Infrastruktur AG\", \"ref:ibnr\": \"8100108\", \"uic_name\": \"Innsbruck Hbf\", \"wikidata\": \"Q668727\", \"platforms\": \"11\", \"wikipedia\": \"de:Innsbruck Hauptbahnhof\", \"wheelchair\": \"yes\", \"railway:ref\": \"I\", \"network:short\": \"VVT\", \"railway:ref:DB\": \"XAI\", \"network:wikidata\": \"Q1668732\", \"public_transport\": \"station\", \"railway:position\": \"75.1 / -0.4\", \"toilets:wheelchair\": \"yes\", \"railway:position:exact\": \"75.130 / -0.434\"},\"boundingbox\":[\"47.2582966\",\"47.2682966\",\"11.3960207\",\"11.4060207\"]}]", QString()});

        ReservationManager mgr;
        Test::clearAll(&mgr);
        QSignalSpy batchChangeSpy(&mgr, &ReservationManager::batchContentChanged);

        Settings settings;
        settings.setQueryLiveData(true);
        ReservationOnlinePostprocessor postProc(&mgr, &settings, [this]() { return &m_nam; });

        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&mgr);
        ImportController importer;
        importer.setReservationManager(&mgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/multi-ticket.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(mgr.batches().size(), 1);

        if (batchChangeSpy.empty()) {
            QVERIFY(batchChangeSpy.wait());
        }
        QCOMPARE(batchChangeSpy.size(), 1);
        QCOMPARE(batchChangeSpy[0][0].toString(), mgr.batches()[0]);

        QCOMPARE(m_nam.requests.size(), 2);
        QVERIFY(m_nam.requests[0].request.url().toString().contains("Wien Hbf"_L1, Qt::CaseInsensitive));
        QVERIFY(m_nam.requests[1].request.url().toString().contains("Innsbruck Hbf"_L1, Qt::CaseInsensitive));

        const auto trip = mgr.reservation(batchChangeSpy[0][0].toString()).value<KItinerary::TrainReservation>().reservationFor().value<KItinerary::TrainTrip>();
        QVERIFY(trip.departureStation().geo().isValid());
        QVERIFY(trip.arrivalStation().geo().isValid());
    }

    void testReverseGeocode()
    {
        ReservationManager mgr;
        Test::clearAll(&mgr);
        QSignalSpy batchChangeSpy(&mgr, &ReservationManager::batchContentChanged);

        QNetworkAccessManager nam;

        Settings settings;
        settings.setQueryLiveData(true);
        ReservationOnlinePostprocessor postProc(&mgr, &settings, [&nam]() { return &nam; });

        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&mgr);
        ImportController importer;
        importer.setReservationManager(&mgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/eurostar-identifier-only.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(mgr.batches().size(), 1);

        QVERIFY(batchChangeSpy.wait(std::chrono::seconds(30)));
        QCOMPARE(batchChangeSpy.size(), 1);
        QCOMPARE(batchChangeSpy[0][0].toString(), mgr.batches()[0]);

        const auto trip = mgr.reservation(batchChangeSpy[0][0].toString()).value<KItinerary::TrainReservation>().reservationFor().value<KItinerary::TrainTrip>();
        qDebug() << trip.departureStation().name() << trip.arrivalStation().name();

        QVERIFY(!trip.departureStation().name().isEmpty());
        QCOMPARE_NE(trip.departureStation().name(), "FRPNO"_L1);
        QCOMPARE(trip.departureStation().address().addressCountry(), u"FR");
        QCOMPARE(trip.departureStation().address().addressLocality(), "Paris"_L1);

        QVERIFY(!trip.arrivalStation().name().isEmpty());
        QCOMPARE_NE(trip.arrivalStation().name(), "DEKOH"_L1);
        QCOMPARE(trip.arrivalStation().address().addressCountry(), u"DE");
        QCOMPARE(trip.arrivalStation().address().addressLocality(), u"Köln");
    }

private:
    MockNetworkAccessManager m_nam;
};

QTEST_GUILESS_MAIN(ReservationOnlinePostprocessorTest)

#include "reservationonlinepostprocessortest.moc"
