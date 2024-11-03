/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "applicationcontroller.h"
#include "favoritelocationmodel.h"
#include "importcontroller.h"
#include "reservationmanager.h"
#include "transfermanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

#include <QAbstractItemModelTester>
#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QtTest/qtest.h>

using namespace Qt::Literals::StringLiterals;

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
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl->commitImport(&importer);
        {
            TripGroupManager mgr;
            QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
            QVERIFY(addSpy.isValid());
            mgr.setReservationManager(&resMgr);
            mgr.setTransferManager(&transferMgr);
            QCOMPARE(addSpy.size(), 1);
            auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
            QCOMPARE(g.elements().size(), resMgr.batches().size());
        }

        TripGroupManager::clear();
        Test::clearAll(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
        {
            TripGroupManager mgr;
            QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
            QVERIFY(addSpy.isValid());
            mgr.setReservationManager(&resMgr);
            mgr.setTransferManager(&transferMgr);
            QCOMPARE(addSpy.size(), 1);
            auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
            QCOMPARE(g.elements().size(), resMgr.batches().size());
        }

        TripGroupManager::clear();
        Test::clearAll(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/multi-traveler-merge-with-countryinfo.json")));
        ctrl->commitImport(&importer);
        {
            TripGroupManager mgr;
            QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
            QVERIFY(addSpy.isValid());
            mgr.setReservationManager(&resMgr);
            mgr.setTransferManager(&transferMgr);
            QCOMPARE(addSpy.size(), 1);
            auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
            QCOMPARE(g.elements().size(), resMgr.batches().size());
        }

        TripGroupManager::clear();
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
        {
            TripGroupManager mgr;
            QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
            QVERIFY(addSpy.isValid());
            mgr.setReservationManager(&resMgr);
            mgr.setTransferManager(&transferMgr);
            QCOMPARE(addSpy.size(), 3);
            QCOMPARE(mgr.tripGroup(addSpy.at(0).at(0).toString()).elements().size(), 2);
            QCOMPARE(mgr.tripGroup(addSpy.at(1).at(0).toString()).elements().size(), 2);
            QCOMPARE(mgr.tripGroup(addSpy.at(2).at(0).toString()).elements().size(), 11);
        }

        Test::clearAll(&resMgr);
        TripGroupManager::clear();
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/tripgroup/time-based-layover-detection.json")));
        ctrl->commitImport(&importer);
        {
            TripGroupManager mgr;
            mgr.setReservationManager(&resMgr);
            mgr.setTransferManager(&transferMgr);
            QCOMPARE(mgr.tripGroups().size(), 1);
            QCOMPARE(mgr.tripGroup(mgr.tripGroups().at(0)).elements().size(), 4);
        }

        Test::clearAll(&resMgr);
        TripGroupManager::clear();
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/tripgroup/unidirectional-train-trip.json")));
        ctrl->commitImport(&importer);
        {
            TripGroupManager mgr;
            mgr.setReservationManager(&resMgr);
            mgr.setTransferManager(&transferMgr);
            QCOMPARE(mgr.tripGroups().size(), 1);
            QCOMPARE(mgr.tripGroup(mgr.tripGroups().at(0)).elements().size(), 2);
        }

        Test::clearAll(&resMgr);
        TripGroupManager::clear();
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/tripgroup/unidirectional-with-events.json")));
        ctrl->commitImport(&importer);
        {
            TripGroupManager mgr;
            mgr.setReservationManager(&resMgr);
            mgr.setTransferManager(&transferMgr);
            QCOMPARE(mgr.tripGroups().size(), 1);
            QCOMPARE(mgr.tripGroup(mgr.tripGroups().at(0)).elements().size(), 4);
        }

        Test::clearAll(&resMgr);
        TripGroupManager::clear();
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/no-arrival-time.json")));
        ctrl->commitImport(&importer);
        {
            TripGroupManager mgr;
            mgr.setReservationManager(&resMgr);
            mgr.setTransferManager(&transferMgr);
            QCOMPARE(mgr.tripGroups().size(), 2);
            auto tg1 = mgr.tripGroup(mgr.tripGroups().at(0));
            auto tg2 = mgr.tripGroup(mgr.tripGroups().at(1));
            if (tg2.name() == "PDL (January 2023)"_L1) {
                std::swap(tg1, tg2);
            }
            qDebug() << tg1.name() << tg2.name();
            QCOMPARE(tg1.elements().size(), 1);
            QCOMPARE(tg2.elements().size(), 2);
        }

        Test::clearAll(&resMgr);
        TripGroupManager::clear();
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/event-multi-day.json")));
        ctrl->commitImport(&importer);
        {
            TripGroupManager mgr;
            mgr.setReservationManager(&resMgr);
            mgr.setTransferManager(&transferMgr);
            QCOMPARE(mgr.tripGroups().size(), 1);
            QCOMPARE(mgr.tripGroup(mgr.tripGroups().at(0)).elements().size(), 1);
        }

        Test::clearAll(&resMgr);
        TripGroupManager::clear();
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/flight-cancelation.json")));
        ctrl->commitImport(&importer);
        {
            TripGroupManager mgr;
            mgr.setReservationManager(&resMgr);
            mgr.setTransferManager(&transferMgr);
            QCOMPARE(mgr.tripGroups().size(), 1);
            QCOMPARE(mgr.tripGroup(mgr.tripGroups().at(0)).elements().size(), 1);
        }
    }

    void testDynamicGrouping()
    {
        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        TripGroupManager::clear();
        TripGroupManager mgr;
        mgr.setReservationManager(&resMgr);
        mgr.setTransferManager(&transferMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ctrl->setTripGroupManager(&mgr);

        // after adding the third element this will find a loop between the two inner legs and remove the first leg as a leading appendix
        // the fourth leg however should be fixing that and result in a single 4 leg group
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/tripgroup/symmetric-two-leg-return-flight.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(mgr.tripGroups().size(), 1);
        auto g = mgr.tripGroup(mgr.tripGroups().at(0));
        QCOMPARE(g.elements().size(), resMgr.batches().size());
    }

    void testChanges()
    {
        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        TripGroupManager mgr;
        mgr.setReservationManager(&resMgr);
        mgr.setTransferManager(&transferMgr);

        QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
        QSignalSpy changeSpy(&mgr, &TripGroupManager::tripGroupChanged);
        QSignalSpy rmSpy(&mgr, &TripGroupManager::tripGroupRemoved);

        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(addSpy.size(), 1);
        auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
        QCOMPARE(g.elements().size(), resMgr.batches().size());
        QCOMPARE(changeSpy.size(), 1);

        changeSpy.clear();
        Test::clearAll(&resMgr);
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(rmSpy.size(), 1);
        QCOMPARE(changeSpy.size(), 1);
    }

    void testGroupName_data()
    {
        QTest::addColumn<QString>("fileName");
        QTest::addColumn<QString>("expectedName");

        QTest::newRow("SFO one way") << QStringLiteral(SOURCE_DIR "/data/google-multi-passenger-flight.json")
                                     << QStringLiteral("San Francisco Airport (March 2017)");
        QTest::newRow("FLR one way multi traveller") << QStringLiteral(SOURCE_DIR "/data/timeline/multi-traveler-merge-with-countryinfo.json")
                                                     << QStringLiteral("Peretola (January 2000)");
        QTest::newRow("Randa") << QStringLiteral(SOURCE_DIR "/../tests/randa2017.json") << QStringLiteral("Randa (September 2017)");
        QTest::newRow("Almeria") << QStringLiteral(SOURCE_DIR "/../tests/akademy2017.json") << QStringLiteral("Almería (July 2017)");
        QTest::newRow("Symmetric") << QStringLiteral(SOURCE_DIR "/data/tripgroup/deutschebahn_two-leg-return.txt.json")
                                   << QStringLiteral("Somewhere(Specific) (November 2027)");
        QTest::newRow("Symmetric, 2 elements") << QStringLiteral(SOURCE_DIR "/data/tripgroup/flight-direct-return.json")
                                               << QStringLiteral("Oslo Airport (June 2018)");
        QTest::newRow("Triangular, different PNR") << QStringLiteral(SOURCE_DIR "/data/tripgroup/train-triangular-different-pnr.json")
                                                   << QStringLiteral("Nürnberg Hbf (February/March 2018)");
        QTest::newRow("Imbalanced roundtrip") << QStringLiteral(SOURCE_DIR "/data/tripgroup/imbalanced-return-trip.json")
                                              << QStringLiteral("Milano Centrale (September 2019)");
        QTest::newRow("IATA BCBP no times") << QStringLiteral(SOURCE_DIR "/data/tripgroup/iata-bcbp-no-times.json")
                                            << QStringLiteral("Milan Malpensa (September 2019)");
        QTest::newRow("Unidirectional") << QStringLiteral(SOURCE_DIR "/data/tripgroup/unidirectional-train-trip.json") << u"Milano Centrale (September 2019)"_s;
        QTest::newRow("Unidirectional with events") << QStringLiteral(SOURCE_DIR "/data/tripgroup/unidirectional-with-events.json")
                                                    << u"KDE Akademy 2024 (September 2024)"_s;
        QTest::newRow("multi-day-event") << QStringLiteral(SOURCE_DIR "/data/timeline/event-multi-day.json") << u"KDE Akademy 2023 (July 2023)"_s;
        QTest::newRow("flight-cancelation") << QStringLiteral(SOURCE_DIR "/data/timeline/flight-cancelation.json")
                                            << u"John F. Kennedy International Airport (October 1996)"_s;
    }

    void testGroupName()
    {
        QFETCH(QString, fileName);
        QFETCH(QString, expectedName);

        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(fileName));
        ctrl->commitImport(&importer);
        TripGroupManager mgr;
        QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
        mgr.setReservationManager(&resMgr);
        mgr.setTransferManager(&transferMgr);
        QCOMPARE(addSpy.size(), 1);
        auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
        QCOMPARE(g.elements().size(), resMgr.batches().size());
        QCOMPARE(g.name(), expectedName);

        // ensure that also every item individually gets a name assigned
        for (const auto &resId : resMgr.batches()) {
            const auto name = mgr.guessName({resId});
            qDebug() << name << resMgr.reservation(resId);
            QVERIFY(!name.isEmpty());
            QVERIFY(!name.startsWith(' '_L1));
            QVERIFY(!name.startsWith('('_L1));
            QVERIFY(!name.contains("/)"_L1));
        }
    }

    void testLeadingAppendixRemoval()
    {
        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QStringLiteral(SOURCE_DIR "/data/tripgroup/leading-appendix.json")));
        ctrl->commitImport(&importer);
        TripGroupManager mgr;
        QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
        mgr.setReservationManager(&resMgr);
        mgr.setTransferManager(&transferMgr);
        QCOMPARE(addSpy.size(), 2);
        auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
        QCOMPARE(g.elements().size(), 1);
        QCOMPARE(g.name(), "Tegel (May 2000)"_L1);
        g = mgr.tripGroup(addSpy.at(1).at(0).toString());
        QCOMPARE(g.elements().size(), resMgr.batches().size() - 1);
        QCOMPARE(g.name(), "Oslo Airport (June 2000)"_L1);
        QCOMPARE(g.slugName(), "oslo-airport-june-2000"_L1);
    }

    void testDeletion()
    {
        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QStringLiteral(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
        TripGroupManager mgr;
        QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
        QSignalSpy aboutToRmSpy(&mgr, &TripGroupManager::tripGroupAboutToBeRemoved);
        QSignalSpy rmSpy(&mgr, &TripGroupManager::tripGroupRemoved);
        mgr.setReservationManager(&resMgr);
        mgr.setTransferManager(&transferMgr);
        QCOMPARE(addSpy.size(), 1);
        QVERIFY(resMgr.batches().size() > 8);
        const auto groupId = addSpy.at(0).at(0).toString();
        const auto g = mgr.tripGroup(groupId);
        mgr.removeReservationsInGroup(groupId);
        QCOMPARE(resMgr.batches().size(), 0);
        QCOMPARE(mgr.tripGroups().size(), 0);

        QCOMPARE(aboutToRmSpy.size(), 1);
        QCOMPARE(aboutToRmSpy.at(0).at(0).toString(), groupId);
        QCOMPARE(aboutToRmSpy.size(), 1);
        QCOMPARE(aboutToRmSpy.at(0).at(0).toString(), groupId);
    }

    void testMerge()
    {
        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);

        TripGroupManager tgMgr;
        tgMgr.setReservationManager(&resMgr);
        tgMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&tgMgr);

        ImportController importer;
        importer.setReservationManager(&resMgr);

        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl->commitImport(&importer);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(tgMgr.tripGroups().size(), 2);

        auto tgId1 = tgMgr.tripGroups()[0];
        auto tgId2 = tgMgr.tripGroups()[1];

        auto mergeId = tgMgr.merge(tgId1, tgId2, u"New Group Name"_s);
        QVERIFY(!mergeId.isEmpty());
        QVERIFY(mergeId == tgId1 || mergeId == tgId2);
        QCOMPARE(tgMgr.tripGroups().size(), 1);

        {
            const auto tg = tgMgr.tripGroup(mergeId);
            QCOMPARE(tg.isAutomaticallyGrouped(), false);
            QCOMPARE(tg.hasAutomaticName(), false);
            QCOMPARE(tg.name(), "New Group Name"_L1);
            QCOMPARE(tg.elements().size(), resMgr.batches().size());
            for (const auto &resId : resMgr.batches()) {
                QCOMPARE(tgMgr.tripGroupIdForReservation(resId), mergeId);
            }
        }

        // manual grouping is persisted and not affected by rescanning
        {
            TripGroupManager tgMgr2;
            tgMgr2.setReservationManager(&resMgr);
            tgMgr2.setTransferManager(&transferMgr);
            QCOMPARE(tgMgr2.tripGroups().size(), 1);
        }

        // no automatic regrouping, regardless of what changes
        for (const auto &res : resMgr.batches()) {
            Q_EMIT resMgr.batchContentChanged(res);
        }
        QCOMPARE(tgMgr.tripGroups().size(), 1);

        while (resMgr.batches().size() > 1) {
            resMgr.removeBatch(resMgr.batches().front());
            QCOMPARE(tgMgr.tripGroups().size(), 1);
            {
                const auto tg = tgMgr.tripGroup(mergeId);
                QCOMPARE(tg.isAutomaticallyGrouped(), false);
                QCOMPARE(tg.hasAutomaticName(), false);
                QCOMPARE(tg.name(), "New Group Name"_L1);
                QCOMPARE(tg.elements().size(), resMgr.batches().size());
            }
        }

        resMgr.removeBatch(resMgr.batches().front());
        QCOMPARE(tgMgr.tripGroups().size(), 0);

        // test adding elements inside a merged group
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl->commitImport(&importer);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);

        tgId1 = tgMgr.tripGroups()[0];
        tgId2 = tgMgr.tripGroups()[1];

        mergeId = tgMgr.merge(tgId1, tgId2, u"New Group 2"_s);
        QVERIFY(!mergeId.isEmpty());
        QVERIFY(mergeId == tgId1 || mergeId == tgId2);
        QCOMPARE(tgMgr.tripGroups().size(), 1);

        QVariantList reservations;
        auto resIds = tgMgr.tripGroup(mergeId).elements();
        resIds.pop_back();
        resIds.pop_front();
        QCOMPARE(resIds.size(), 11);
        for (const auto &resId : resIds) {
            reservations.push_back(resMgr.reservation(resId));
            resMgr.removeBatch(resId);
        }
        QCOMPARE(tgMgr.tripGroups().size(), 1);

        for (const auto &res : reservations) {
            resMgr.addReservation(res);
        }
        QCOMPARE(tgMgr.tripGroups().size(), 1);
        QCOMPARE(tgMgr.tripGroup(mergeId).name(), "New Group 2"_L1);
    }

    void testSplitFront()
    {
        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);

        TripGroupManager tgMgr;
        tgMgr.setReservationManager(&resMgr);
        tgMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&tgMgr);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(tgMgr.tripGroups().size(), 1);

        QStringList elements({resMgr.batches()[0], resMgr.batches()[1], resMgr.batches()[2]});
        const auto tgId = tgMgr.createGroup(elements, u"New Group"_s);
        QVERIFY(!tgId.isEmpty());
        QCOMPARE(tgMgr.tripGroups().size(), 2);

        auto tg = tgMgr.tripGroup(tgId);
        QCOMPARE(tg.isAutomaticallyGrouped(), false);
        QCOMPARE(tg.hasAutomaticName(), false);
        QCOMPARE(tg.name(), "New Group"_L1);
        QCOMPARE(tg.elements().size(), 3);

        const auto tgId2 = tgMgr.tripGroups()[0] == tgId ? tgMgr.tripGroups()[1] : tgMgr.tripGroups()[0];
        tg = tgMgr.tripGroup(tgId2);
        QCOMPARE(tg.isAutomaticallyGrouped(), false);
        QCOMPARE(tg.hasAutomaticName(), true);
        QCOMPARE(tg.name(), "Randa (September 2017)"_L1);
        QCOMPARE(tg.elements().size(), 8);

        // manual grouping is persisted and not affected by rescanning
        {
            TripGroupManager tgMgr2;
            tgMgr2.setReservationManager(&resMgr);
            tgMgr2.setTransferManager(&transferMgr);
            QCOMPARE(tgMgr2.tripGroups().size(), 2);
            tg = tgMgr2.tripGroup(tgId);
            QCOMPARE(tg.elements().size(), 3);
            tg = tgMgr2.tripGroup(tgId2);
            QCOMPARE(tg.elements().size(), 8);
        }
    }

    void testSplitBack()
    {
        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);

        TripGroupManager tgMgr;
        tgMgr.setReservationManager(&resMgr);
        tgMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&tgMgr);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(tgMgr.tripGroups().size(), 1);
        QCOMPARE(resMgr.batches().size(), 11);

        QStringList elements({resMgr.batches()[8], resMgr.batches()[9], resMgr.batches()[10]});
        const auto tgId = tgMgr.createGroup(elements, u"New Group"_s);
        QVERIFY(!tgId.isEmpty());
        QCOMPARE(tgMgr.tripGroups().size(), 2);

        auto tg = tgMgr.tripGroup(tgId);
        QCOMPARE(tg.isAutomaticallyGrouped(), false);
        QCOMPARE(tg.hasAutomaticName(), false);
        QCOMPARE(tg.name(), "New Group"_L1);
        QCOMPARE(tg.elements().size(), 3);

        const auto tgId2 = tgMgr.tripGroups()[0] == tgId ? tgMgr.tripGroups()[1] : tgMgr.tripGroups()[0];
        tg = tgMgr.tripGroup(tgId2);
        QCOMPARE(tg.isAutomaticallyGrouped(), false);
        QCOMPARE(tg.hasAutomaticName(), true);
        QCOMPARE(tg.name(), "Randa (September 2017)"_L1);
        QCOMPARE(tg.elements().size(), 8);

        tg.setIsAutomaticallyGrouped(true);
        tgMgr.updateTripGroup(tgId2, tg);

        // manual grouping is persisted and not affected by rescanning, even if the leading part is automatically grouped
        {
            TripGroupManager tgMgr2;
            tgMgr2.setReservationManager(&resMgr);
            tgMgr2.setTransferManager(&transferMgr);
            QCOMPARE(tgMgr2.tripGroups().size(), 2);
            tg = tgMgr2.tripGroup(tgId);
            QCOMPARE(tg.elements().size(), 3);
            tg = tgMgr2.tripGroup(tgId2);
            QCOMPARE(tg.elements().size(), 8);
        }
    }

    void testExplicitGroupCreation()
    {
        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);

        TripGroupManager tgMgr;
        tgMgr.setReservationManager(&resMgr);
        tgMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&tgMgr);

        ImportController importer;
        QAbstractItemModelTester modelTest(&importer);
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));

        // create new group from the first half
        for (int i = importer.rowCount() / 2; i < importer.rowCount(); ++i) {
            QVERIFY(importer.setData(importer.index(i, 0), false, ImportController::SelectedRole));
        }
        importer.setTripGroupName(u"Custom Group Name"_s);
        QCOMPARE(importer.rowCount(), 11);
        QCOMPARE(importer.hasSelectedReservation(), true);
        QCOMPARE(importer.selectedReservations().size(), 5);

        ctrl->commitImport(&importer);
        QCOMPARE(resMgr.batches().size(), 5);
        QCOMPARE(tgMgr.tripGroups().size(), 1);
        const auto tgId = tgMgr.tripGroups().at(0);
        QVERIFY(!tgId.isEmpty());
        auto tg = tgMgr.tripGroup(tgId);
        QCOMPARE(tg.elements().size(), 5);
        QCOMPARE(tg.name(), "Custom Group Name"_L1);
        QCOMPARE(tg.isAutomaticallyGrouped(), false);
        QCOMPARE(tg.hasAutomaticName(), false);

        // add the second half to that group
        QCOMPARE(importer.rowCount(), 6);
        QCOMPARE(importer.hasSelectedReservation(), false);
        for (int i = 0; i < importer.rowCount(); ++i) {
            QVERIFY(importer.setData(importer.index(i, 0), true, ImportController::SelectedRole));
        }
        QCOMPARE(importer.hasSelectedReservation(), true);
        QCOMPARE(importer.selectedReservations().size(), 6);
        QCOMPARE(importer.tripGroupName(), QString());
        importer.setTripGroupId(tgId);

        ctrl->commitImport(&importer);
        QCOMPARE(resMgr.batches().size(), 11);
        QCOMPARE(tgMgr.tripGroups().size(), 1);
        tg = tgMgr.tripGroup(tgId);
        QCOMPARE(tg.elements().size(), 11);
        QCOMPARE(tg.name(), "Custom Group Name"_L1);
        QCOMPARE(tg.isAutomaticallyGrouped(), false);
        QCOMPARE(tg.hasAutomaticName(), false);
    }

    void testEmptyGroup()
    {
        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);

        TripGroupManager::clear();
        TripGroupManager tgMgr;
        tgMgr.setReservationManager(&resMgr);
        tgMgr.setTransferManager(&transferMgr);

        QCOMPARE(tgMgr.tripGroups().size(), 0);

        const auto tgId = tgMgr.createEmptyGroup(u"My New Trip"_s);
        QVERIFY(!tgId.isEmpty());
        QCOMPARE(tgMgr.tripGroups().size(), 1);
        QCOMPARE(tgMgr.tripGroups().at(0), tgId);
        const auto tg = tgMgr.tripGroup(tgId);
        QCOMPARE(tg.hasAutomaticName(), false);
        QCOMPARE(tg.name(), "My New Trip"_L1);
        QCOMPARE(tg.beginDateTime().isValid(), false);
        QCOMPARE(tg.endDateTime().isValid(), false);

        {
            // check the consistency check on load doesn't clean this up
            TripGroupManager tgMgr;
            QCOMPARE(tgMgr.tripGroups().size(), 1); // after loading, before consistency checks
            tgMgr.setReservationManager(&resMgr);
            tgMgr.setTransferManager(&transferMgr); // after consistency checks
            QCOMPARE(tgMgr.tripGroups().size(), 1);
            QCOMPARE(tgMgr.tripGroups().at(0), tgId);
            const auto tg = tgMgr.tripGroup(tgId);
            QCOMPARE(tg.hasAutomaticName(), false);
            QCOMPARE(tg.name(), "My New Trip"_L1);
            QCOMPARE(tg.beginDateTime().isValid(), false);
            QCOMPARE(tg.endDateTime().isValid(), false);
        }

        tgMgr.removeReservationsInGroup(tgId);
        QCOMPARE(tgMgr.tripGroups().size(), 0);

        {
            // check the consistency check on load doesn't clean this up
            TripGroupManager tgMgr;
            QCOMPARE(tgMgr.tripGroups().size(), 0);
            tgMgr.setReservationManager(&resMgr);
            tgMgr.setTransferManager(&transferMgr);
            QCOMPARE(tgMgr.tripGroups().size(), 0);
        }
    }

    void testMultiGroupBackup()
    {
        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ctrl->setTransferManager(&transferMgr);

        TripGroupManager tgMgr;
        tgMgr.setReservationManager(&resMgr);
        tgMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&tgMgr);

        FavoriteLocationModel favLoc;
        ctrl->setFavoriteLocationModel(&favLoc);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(tgMgr.tripGroups().size(), 1);

        QStringList elements({resMgr.batches()[0], resMgr.batches()[1], resMgr.batches()[2]});
        const auto tgId = tgMgr.createGroup(elements, u"New Group"_s);
        QVERIFY(!tgId.isEmpty());
        QCOMPARE(tgMgr.tripGroups().size(), 2);

        auto tg = tgMgr.tripGroup(tgId);
        QCOMPARE(tg.isAutomaticallyGrouped(), false);
        QCOMPARE(tg.hasAutomaticName(), false);
        QCOMPARE(tg.name(), "New Group"_L1);
        QCOMPARE(tg.elements().size(), 3);

        const auto tgId2 = tgMgr.tripGroups()[0] == tgId ? tgMgr.tripGroups()[1] : tgMgr.tripGroups()[0];
        tg = tgMgr.tripGroup(tgId2);
        tg.setIsAutomaticallyGrouped(false);
        tg.setNameIsAutomatic(false);
        tg.setName(u"Group 2"_s);
        tg.setMatrixRoomId(u"#group2:kde.org"_s);
        tgMgr.updateTripGroup(tgId2, tg);

        QTemporaryFile tmp;
        QVERIFY(tmp.open());
        tmp.close();
        ctrl->exportToFile(QUrl::fromLocalFile(tmp.fileName()));

        Test::clearAll(&resMgr);
        TripGroupManager::clear();
        QCOMPARE(resMgr.batches().size(), 0);
        QCOMPARE(tgMgr.tripGroups().size(), 0);

        importer.importFromUrl(QUrl::fromLocalFile(tmp.fileName()));
        ctrl->commitImport(&importer);
        QCOMPARE(resMgr.batches().size(), 11);
        QCOMPARE(tgMgr.tripGroups().size(), 2);

        const auto newTg1 = tgMgr.tripGroup(tgMgr.tripGroups()[0]).beginDateTime() < tgMgr.tripGroup(tgMgr.tripGroups()[1]).beginDateTime()
            ? tgMgr.tripGroup(tgMgr.tripGroups()[0])
            : tgMgr.tripGroup(tgMgr.tripGroups()[1]);
        const auto newTg2 = tgMgr.tripGroup(tgMgr.tripGroups()[1]).beginDateTime() < tgMgr.tripGroup(tgMgr.tripGroups()[0]).beginDateTime()
            ? tgMgr.tripGroup(tgMgr.tripGroups()[0])
            : tgMgr.tripGroup(tgMgr.tripGroups()[1]);

        QCOMPARE(newTg1.elements().size(), 3);
        QCOMPARE(newTg1.name(), "New Group"_L1);
        QCOMPARE(newTg1.isAutomaticallyGrouped(), false);
        QCOMPARE(newTg1.hasAutomaticName(), false);

        QCOMPARE(newTg2.elements().size(), 8);
        QCOMPARE(newTg2.name(), "Group 2"_L1);
        QCOMPARE(newTg2.matrixRoomId(), "#group2:kde.org"_L1);
        QCOMPARE(newTg2.isAutomaticallyGrouped(), false);
        QCOMPARE(newTg2.hasAutomaticName(), false);
    }

    void testIncrementalBatchImport()
    {
        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        TripGroupManager mgr;
        mgr.setReservationManager(&resMgr);
        mgr.setTransferManager(&transferMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ctrl->setTripGroupManager(&mgr);
        ImportController importer;
        importer.setReservationManager(&resMgr);

        const auto data = QJsonDocument::fromJson(Test::readFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json"))).array();
        QCOMPARE(data.size(), 4);
        importer.importData(QJsonDocument(data[0].toObject()).toJson());
        QCOMPARE(importer.rowCount(), 1);
        ctrl->commitImport(&importer);

        QCOMPARE(resMgr.batches().size(), 1);
        QCOMPARE(mgr.tripGroups().size(), 1);
        auto tg = mgr.tripGroup(mgr.tripGroups()[0]);
        QCOMPARE(tg.elements().size(), 1);
        QVERIFY(!tg.name().isEmpty());

        importer.importData(QJsonDocument(data[2].toObject()).toJson());
        importer.setTripGroupId(mgr.tripGroups()[0]);
        QCOMPARE(importer.rowCount(), 1);
        ctrl->commitImport(&importer);

        QCOMPARE(resMgr.batches().size(), 1);
        QCOMPARE(mgr.tripGroups().size(), 1);
        tg = mgr.tripGroup(mgr.tripGroups()[0]);
        QCOMPARE(tg.elements().size(), 1);
        QVERIFY(!tg.name().isEmpty());
    }
};
QTEST_GUILESS_MAIN(TripGroupTest)

#include "tripgrouptest.moc"
