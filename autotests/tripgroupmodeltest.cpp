/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "applicationcontroller.h"
#include "importcontroller.h"
#include "locationhelper.h"
#include "reservationmanager.h"
#include "transfermanager.h"
#include "tripgroup.h"
#include "tripgroupfilterproxymodel.h"
#include "tripgroupmanager.h"
#include "tripgroupmodel.h"

#include <KItinerary/LocationUtil>

#include <QAbstractItemModelTester>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

using namespace Qt::Literals;
using namespace KItinerary;

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("LANG", "C");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class TripGroupModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void init()
    {
        TripGroupManager::clear();
    }

    void testModel()
    {
        TripGroupModel model;
        model.setCurrentDateTime(QDateTime{{2017, 3 , 4}, {15, 0}, QTimeZone("America/New_York")});
        QAbstractItemModelTester modelTest(&model);
        QSignalSpy insertSpy(&model, &QAbstractItemModel::rowsInserted);
        QSignalSpy updateSpy(&model, &QAbstractItemModel::dataChanged);
        QSignalSpy removeSpy(&model, &QAbstractItemModel::rowsRemoved);

        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);

        TripGroupManager tgMgr;
        tgMgr.setReservationManager(&resMgr);
        tgMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&tgMgr);
        model.setTripGroupManager(&tgMgr);

        ImportController importer;
        importer.setReservationManager(&resMgr);

        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(insertSpy.size(), 1);
        auto idx = model.index(0, 0);
        QCOMPARE(idx.data(Qt::DisplayRole).toString(), "San Francisco Airport (March 2017)"_L1);
        QCOMPARE(idx.data(TripGroupModel::PositionRole).toInt(), TripGroupModel::Current);

        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(insertSpy.size(), 2);
        QCOMPARE(insertSpy.back().at(1).toInt(), 0);
        idx = model.index(0, 0);
        QCOMPARE(idx.data(Qt::DisplayRole).toString(), "Randa (September 2017)"_L1);
        QCOMPARE(idx.data(TripGroupModel::PositionRole).toInt(), TripGroupModel::Future);
        const auto tgId = idx.data(TripGroupModel::TripGroupIdRole).toString();
        QVERIFY(!tgId.isEmpty());

        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/multi-traveler-merge-with-countryinfo.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(model.rowCount(), 3);
        QCOMPARE(insertSpy.size(), 3);
        QCOMPARE(insertSpy.back().at(1).toInt(), 2);
        idx = model.index(2, 0);
        QCOMPARE(idx.data(Qt::DisplayRole).toString(), "Peretola (January 2000)"_L1);
        QCOMPARE(idx.data(TripGroupModel::PositionRole).toInt(), TripGroupModel::Past);

        insertSpy.clear();

        auto tg = tgMgr.tripGroup(tgId);
        tg.setName(u"KDE Randa Meeting 2017"_s);
        tg.setNameIsAutomatic(false);
        tgMgr.updateTripGroup(tgId, tg);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(updateSpy.back().at(0).toModelIndex().row(), 0);
        idx = model.index(0, 0);
        QCOMPARE(idx.data(Qt::DisplayRole).toString(), "KDE Randa Meeting 2017"_L1);

        updateSpy.clear();

        tgMgr.removeReservationsInGroup(tgId);
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(removeSpy.size(), 1);
        QCOMPARE(removeSpy.back().at(1).toInt(), 0);

        QCOMPARE(insertSpy.size(), 0);
        QCOMPARE(updateSpy.size(), 0);
    }

    void testAdjacency()
    {
        TripGroupModel model;
        model.setCurrentDateTime(QDateTime{{2017, 3 , 4}, {15, 0}, QTimeZone("America/New_York")});
        QAbstractItemModelTester modelTest(&model);

        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);

        TripGroupManager tgMgr;
        tgMgr.setReservationManager(&resMgr);
        tgMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&tgMgr);
        model.setTripGroupManager(&tgMgr);

        ImportController importer;
        importer.setReservationManager(&resMgr);

        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl->commitImport(&importer);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/multi-traveler-merge-with-countryinfo.json")));
        ctrl->commitImport(&importer);

        QCOMPARE(model.rowCount(), 3);

        TripGroupFilterProxyModel proxyModel;
        QAbstractItemModelTester proxyModelTester(&proxyModel);
        proxyModel.setSourceModel(&model);

        auto adjacent = model.adjacentTripGroups(model.index(0, 0).data(TripGroupModel::TripGroupIdRole).toString());
        QCOMPARE(adjacent.size(), 1);
        QCOMPARE(adjacent[0], model.index(1, 0).data(TripGroupModel::TripGroupIdRole).toString());
        proxyModel.setFilteredGroupIds(adjacent);
        QCOMPARE(proxyModel.rowCount(), 1);
        QCOMPARE(proxyModel.index(0, 0).data(TripGroupModel::TripGroupIdRole).toString(), model.index(1, 0).data(TripGroupModel::TripGroupIdRole).toString());

        adjacent = model.adjacentTripGroups(model.index(1, 0).data(TripGroupModel::TripGroupIdRole).toString());
        QCOMPARE(adjacent.size(), 2);
        proxyModel.setFilteredGroupIds(adjacent);
        QCOMPARE(proxyModel.rowCount(), 2);
        QCOMPARE(proxyModel.index(0, 0).data(TripGroupModel::TripGroupIdRole).toString(), model.index(0, 0).data(TripGroupModel::TripGroupIdRole).toString());
        QCOMPARE(proxyModel.index(1, 0).data(TripGroupModel::TripGroupIdRole).toString(), model.index(2, 0).data(TripGroupModel::TripGroupIdRole).toString());

        adjacent = model.adjacentTripGroups(model.index(2, 0).data(TripGroupModel::TripGroupIdRole).toString());
        QCOMPARE(adjacent.size(), 1);
        QCOMPARE(adjacent[0], model.index(1, 0).data(TripGroupModel::TripGroupIdRole).toString());
        proxyModel.setFilteredGroupIds(adjacent);
        QCOMPARE(proxyModel.rowCount(), 1);
        QCOMPARE(proxyModel.index(0, 0).data(TripGroupModel::TripGroupIdRole).toString(), model.index(1, 0).data(TripGroupModel::TripGroupIdRole).toString());
    }

    void testIntersection()
    {
        TripGroupModel model;
        model.setCurrentDateTime(QDateTime{{2017, 3 , 4}, {15, 0}, QTimeZone("America/New_York")});
        QAbstractItemModelTester modelTest(&model);

        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);

        TripGroupManager tgMgr;
        tgMgr.setReservationManager(&resMgr);
        tgMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&tgMgr);
        model.setTripGroupManager(&tgMgr);

        ImportController importer;
        importer.setReservationManager(&resMgr);

        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl->commitImport(&importer);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/multi-traveler-merge-with-countryinfo.json")));
        ctrl->commitImport(&importer);

        QCOMPARE(model.rowCount(), 3);

        auto intersecting = model.intersectingTripGroups({{2024, 7, 7}, {13, 0}}, {{2024, 7, 7}, {14, 0}});
        QCOMPARE(intersecting.size(), 0);
        intersecting = model.intersectingTripGroups({{1924, 7, 7}, {13, 0}}, {{1924, 7, 7}, {14, 0}});
        QCOMPARE(intersecting.size(), 0);
        intersecting = model.intersectingTripGroups({{2017, 7, 7}, {13, 0}}, {{2017, 7, 7}, {14, 0}});
        QCOMPARE(intersecting.size(), 0);

        intersecting = model.intersectingTripGroups({{2017, 9, 15}, {13, 0}}, {{2017, 9, 15}, {14, 0}});
        QCOMPARE(intersecting.size(), 1);
        QCOMPARE(intersecting[0], model.index(0, 0).data(TripGroupModel::TripGroupIdRole).toString());
        intersecting = model.intersectingTripGroups({{2017, 8, 15}, {13, 0}}, {{2017, 9, 15}, {14, 0}});
        QCOMPARE(intersecting.size(), 1);
        QCOMPARE(intersecting[0], model.index(0, 0).data(TripGroupModel::TripGroupIdRole).toString());
        intersecting = model.intersectingTripGroups({{2017, 9, 15}, {13, 0}}, {{2017, 10, 15}, {14, 0}});
        QCOMPARE(intersecting.size(), 1);
        QCOMPARE(intersecting[0], model.index(0, 0).data(TripGroupModel::TripGroupIdRole).toString());
        intersecting = model.intersectingTripGroups({{2017, 8, 15}, {13, 0}}, {{2017, 10, 15}, {14, 0}});
        QCOMPARE(intersecting.size(), 1);
        QCOMPARE(intersecting[0], model.index(0, 0).data(TripGroupModel::TripGroupIdRole).toString());

        intersecting = model.intersectingTripGroups({{2017, 3, 4}, {13, 0}}, {{2017, 3, 4}, {14, 0}, QTimeZone("America/New_York")});
        QCOMPARE(intersecting.size(), 1);
        QCOMPARE(intersecting[0], model.index(1, 0).data(TripGroupModel::TripGroupIdRole).toString());
        intersecting = model.intersectingTripGroups({{2017, 3, 4}, {13, 0}}, {{2017, 4, 4}, {14, 0}, QTimeZone("America/New_York")});
        QCOMPARE(intersecting.size(), 1);
        QCOMPARE(intersecting[0], model.index(1, 0).data(TripGroupModel::TripGroupIdRole).toString());
        intersecting = model.intersectingTripGroups({{2017, 2, 4}, {13, 0}}, {{2017, 3, 4}, {14, 0}, QTimeZone("America/New_York")});
        QCOMPARE(intersecting.size(), 1);
        QCOMPARE(intersecting[0], model.index(1, 0).data(TripGroupModel::TripGroupIdRole).toString());
        intersecting = model.intersectingTripGroups({{2017, 2, 4}, {13, 0}}, {{2017, 4, 4}, {14, 0}, QTimeZone("America/New_York")});
        QCOMPARE(intersecting.size(), 1);
        QCOMPARE(intersecting[0], model.index(1, 0).data(TripGroupModel::TripGroupIdRole).toString());

        intersecting = model.intersectingTripGroups({{2000, 1, 1}, {13, 0}}, {{2000, 1, 1}, {14, 0}});
        QCOMPARE(intersecting.size(), 1);
        QCOMPARE(intersecting[0], model.index(2, 0).data(TripGroupModel::TripGroupIdRole).toString());
        intersecting = model.intersectingTripGroups({{1999, 1, 1}, {13, 0}}, {{2000, 1, 1}, {14, 0}});
        QCOMPARE(intersecting.size(), 1);
        QCOMPARE(intersecting[0], model.index(2, 0).data(TripGroupModel::TripGroupIdRole).toString());
        intersecting = model.intersectingTripGroups({{1999, 1, 1}, {13, 0}}, {{2000, 2, 1}, {14, 0}});
        QCOMPARE(intersecting.size(), 1);
        QCOMPARE(intersecting[0], model.index(2, 0).data(TripGroupModel::TripGroupIdRole).toString());
        intersecting = model.intersectingTripGroups({{2000, 1, 1}, {13, 0}}, {{2000, 1, 2}, {14, 0}});
        QCOMPARE(intersecting.size(), 1);
        QCOMPARE(intersecting[0], model.index(2, 0).data(TripGroupModel::TripGroupIdRole).toString());

        intersecting = model.intersectingTripGroups({{2017, 1, 1}, {13, 0}}, {{2018, 1, 1}, {14, 0}});
        QCOMPARE(intersecting.size(), 2);
        intersecting = model.intersectingTripGroups({{2017, 1, 1}, {13, 0}}, {{2017, 9, 15}, {14, 0}});
        QCOMPARE(intersecting.size(), 2);
    }

    void testCurrentBatch()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);

        TransferManager transferMgr;

        TripGroupManager mgr;
        mgr.setReservationManager(&resMgr);
        mgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&mgr);

        TripGroupModel model;
        model.setCurrentDateTime(QDateTime({2017, 8, 1}, {23, 0}, QTimeZone("Europe/Zurich")));
        QAbstractItemModelTester tester(&model);
        model.setTripGroupManager(&mgr);

        QSignalSpy currentResChangedSpy(&model, &TripGroupModel::currentBatchChanged);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(model.rowCount(), 1);
        QVERIFY(!currentResChangedSpy.empty());
        const auto tg = mgr.tripGroup(mgr.tripGroups().at(0));
        QCOMPARE(tg.elements().size(), 11);

        model.setCurrentDateTime(QDateTime({2017, 8, 1}, {23, 0}, QTimeZone("Europe/Zurich")));
        QVERIFY(model.currentBatchId().isEmpty());

        model.setCurrentDateTime(QDateTime({2017, 9, 9}, {23, 0}, QTimeZone("Europe/Zurich")));
        QVERIFY(!model.currentBatchId().isEmpty());
        QCOMPARE(model.currentBatchId(), tg.elements().at(0));

        model.setCurrentDateTime(QDateTime({2017, 9, 10}, {14, 0}, QTimeZone("Europe/Zurich")));
        QVERIFY(!model.currentBatchId().isEmpty());
        QCOMPARE(model.currentBatchId(), tg.elements().at(1));

        model.setCurrentDateTime(QDateTime({2017, 9, 10}, {14, 5}, QTimeZone("Europe/Zurich")));
        QVERIFY(!model.currentBatchId().isEmpty());
        QCOMPARE(model.currentBatchId(), tg.elements().at(2));

        model.setCurrentDateTime(QDateTime({2017, 9, 10}, {20, 0}, QTimeZone("Europe/Zurich")));
        QVERIFY(model.currentBatchId().isEmpty());

        model.setCurrentDateTime(QDateTime({2019, 1, 1}, {0, 0}, QTimeZone("Europe/Zurich")));
        QVERIFY(model.currentBatchId().isEmpty());
    }

    void testLocationAtTime()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);

        TransferManager transferMgr;

        TripGroupManager mgr;
        mgr.setReservationManager(&resMgr);
        mgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&mgr);

        TripGroupModel model;
        model.setCurrentDateTime(QDateTime({2017, 8, 1}, {23, 0}, QTimeZone("Europe/Zurich")));
        QAbstractItemModelTester tester(&model);
        model.setTripGroupManager(&mgr);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl->commitImport(&importer);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/multi-traveler-merge-with-countryinfo.json")));
        ctrl->commitImport(&importer);
        // test data puts our known location to DE-BY and then adds a hotel in DE-BE for the BY-only public holiday on 2022-06-16
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/bug455083.json")));
        ctrl->commitImport(&importer);

        QCOMPARE(model.rowCount(), 5);

        QCOMPARE(model.locationAtTime(QDateTime({1970, 1, 1}, {})), QVariant());

        QCOMPARE(LocationUtil::name(model.locationAtTime(QDateTime({2017, 9, 10}, {8, 30}, QTimeZone("Europe/Zurich")))), u"Zürich");
        QCOMPARE(LocationUtil::name(model.locationAtTime(QDateTime({2017, 9, 10}, {15, 30}, QTimeZone("Europe/Zurich")))), u"Randa");
        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2017, 9, 11}, {0, 0}))), "CH-VS"_L1);
        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2017, 9, 16}, {0, 0}))), "DE-BE"_L1);

        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2022, 6, 12}, {0, 0}, QTimeZone("Europe/Berlin")))), "DE-BY"_L1);
        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2022, 6, 13}, {0, 0}, QTimeZone("Europe/Berlin")))), "DE-BY"_L1);
        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2022, 6, 14}, {0, 0}, QTimeZone("Europe/Berlin")))), "DE-BE"_L1);
        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2022, 6, 16}, {0, 0}, QTimeZone("Europe/Berlin")))), "DE-BE"_L1);
        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2022, 6, 17}, {0, 0}, QTimeZone("Europe/Berlin")))), "DE-BY"_L1);
        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2022, 6, 18}, {0, 0}, QTimeZone("Europe/Berlin")))), "DE-BY"_L1);
    }
};
QTEST_GUILESS_MAIN(TripGroupModelTest)

#include "tripgroupmodeltest.moc"
