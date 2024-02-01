/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "applicationcontroller.h"
#include "reservationmanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("LANG", "C");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class TripGroupTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qputenv("LC_ALL", "en_US.utf-8");
        QStandardPaths::setTestModeEnabled(true);
    }

    void init()
    {
        TripGroupManager::clear();
    }

    void testGrouping()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        {
            TripGroupManager mgr;
            QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
            QVERIFY(addSpy.isValid());
            mgr.setReservationManager(&resMgr);
            QCOMPARE(addSpy.size(), 1);
            auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
            QCOMPARE(g.elements().size(), resMgr.batches().size());
        }

        TripGroupManager::clear();
        Test::clearAll(&resMgr);
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        {
            TripGroupManager mgr;
            QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
            QVERIFY(addSpy.isValid());
            mgr.setReservationManager(&resMgr);
            QCOMPARE(addSpy.size(), 1);
            auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
            QCOMPARE(g.elements().size(), resMgr.batches().size());
        }

        TripGroupManager::clear();
        Test::clearAll(&resMgr);
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/multi-traveler-merge-with-countryinfo.json")));
        {
            TripGroupManager mgr;
            QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
            QVERIFY(addSpy.isValid());
            mgr.setReservationManager(&resMgr);
            QCOMPARE(addSpy.size(), 1);
            auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
            QCOMPARE(g.elements().size(), resMgr.batches().size());
        }

        TripGroupManager::clear();
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        {
            TripGroupManager mgr;
            QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
            QVERIFY(addSpy.isValid());
            mgr.setReservationManager (&resMgr);
            QCOMPARE(addSpy.size(), 3);
            QCOMPARE(mgr.tripGroup(addSpy.at(0).at(0).toString()).elements().size(), 2);
            QCOMPARE(mgr.tripGroup(addSpy.at(1).at(0).toString()).elements().size(), 2);
            QCOMPARE(mgr.tripGroup(addSpy.at(2).at(0).toString()).elements().size(), 11);
        }

        Test::clearAll(&resMgr);
        TripGroupManager::clear();
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/tripgroup/time-based-layover-detection.json")));
        {
            TripGroupManager mgr;
            mgr.setReservationManager (&resMgr);
            QCOMPARE(mgr.tripGroups().size(), 1);
            QCOMPARE(mgr.tripGroup(mgr.tripGroups().at(0)).elements().size(), 4);
        }
    }

    void testDynamicGrouping()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        TripGroupManager::clear();
        TripGroupManager mgr;
        mgr.setReservationManager(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);

        // after adding the third element this will find a loop between the two inner legs and remove the first leg as a leading appendix
        // the fourth leg however should be fixing that and result in a single 4 leg group
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/tripgroup/symmetric-two-leg-return-flight.json")));
        QCOMPARE(mgr.tripGroups().size(), 1);
        auto g = mgr.tripGroup(mgr.tripGroups().at(0));
        QCOMPARE(g.elements().size(), resMgr.batches().size());
    }

    void testChanges()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        TripGroupManager mgr;
        mgr.setReservationManager(&resMgr);

        QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
        QSignalSpy changeSpy(&mgr, &TripGroupManager::tripGroupChanged);
        QSignalSpy rmSpy(&mgr, &TripGroupManager::tripGroupRemoved);

        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        QCOMPARE(addSpy.size(), 1);
        auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
        QCOMPARE(g.elements().size(), resMgr.batches().size());
        QCOMPARE(changeSpy.size(), 0);

        changeSpy.clear();
        Test::clearAll(&resMgr);
        QCOMPARE(rmSpy.size(), 1);
        QCOMPARE(changeSpy.size(), 0);
    }

    void testGroupName_data()
    {
        QTest::addColumn<QString>("fileName");
        QTest::addColumn<QString>("expectedName");

        QTest::newRow("SFO one way") << QStringLiteral(SOURCE_DIR "/data/google-multi-passenger-flight.json") << QStringLiteral("San Francisco Airport (March 2017)");
        QTest::newRow("FLR one way multi traveller") << QStringLiteral(SOURCE_DIR "/data/timeline/multi-traveler-merge-with-countryinfo.json") << QStringLiteral("Peretola (January 2000)");
        QTest::newRow("Randa") << QStringLiteral(SOURCE_DIR "/../tests/randa2017.json") << QStringLiteral("Randa (September 2017)");
        QTest::newRow("Almeria") << QStringLiteral(SOURCE_DIR "/../tests/akademy2017.json") << QStringLiteral("Almería (July 2017)");
        QTest::newRow("Symmetric") << QStringLiteral(SOURCE_DIR "/data/tripgroup/deutschebahn_two-leg-return.txt.json") << QStringLiteral("Somewhere(Specific) (November 2027)");
        QTest::newRow("Symmetric, 2 elements") << QStringLiteral(SOURCE_DIR "/data/tripgroup/flight-direct-return.json") << QStringLiteral("Oslo Airport (June 2018)");
        QTest::newRow("Triangular, different PNR") << QStringLiteral(SOURCE_DIR "/data/tripgroup/train-triangular-different-pnr.json") << QStringLiteral("Nürnberg Hbf (February/March 2018)");
        QTest::newRow("Imbalanced roundtrip") << QStringLiteral(SOURCE_DIR "/data/tripgroup/imbalanced-return-trip.json") << QStringLiteral("Milano Centrale (September 2019)");
        QTest::newRow("IATA BCBP no times") << QStringLiteral(SOURCE_DIR "/data/tripgroup/iata-bcbp-no-times.json") << QStringLiteral("Milan Malpensa (September 2019)");
    }

    void testGroupName()
    {
        QFETCH(QString, fileName);
        QFETCH(QString, expectedName);

        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ctrl->importFromUrl(QUrl::fromLocalFile(fileName));
        TripGroupManager mgr;
        QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
        mgr.setReservationManager(&resMgr);
        QCOMPARE(addSpy.size(), 1);
        auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
        QCOMPARE(g.elements().size(), resMgr.batches().size());
        QCOMPARE(g.name(), expectedName);
    }

    void testLeadingAppendixRemoval()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ctrl->importFromUrl(QUrl::fromLocalFile(QStringLiteral(SOURCE_DIR "/data/tripgroup/leading-appendix.json")));
        TripGroupManager mgr;
        QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
        mgr.setReservationManager(&resMgr);
        QCOMPARE(addSpy.size(), 1);
        auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
        QCOMPARE(g.elements().size(), resMgr.batches().size() - 1);
        QCOMPARE(g.name(), QStringLiteral("Oslo Airport (June 2000)"));
    }

    void testDeletion()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ctrl->importFromUrl(QUrl::fromLocalFile(QStringLiteral(SOURCE_DIR "/../tests/randa2017.json")));
        TripGroupManager mgr;
        QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
        mgr.setReservationManager(&resMgr);
        QCOMPARE(addSpy.size(), 1);
        QVERIFY(resMgr.batches().size() > 8);
        const auto groupId = addSpy.at(0).at(0).toString();
        const auto g = mgr.tripGroup(groupId);
        mgr.removeReservationsInGroup(groupId);
        QCOMPARE(resMgr.batches().size(), 0);
        QCOMPARE(mgr.tripGroups().size(), 0);
    }
};
QTEST_GUILESS_MAIN(TripGroupTest)

#include "tripgrouptest.moc"
