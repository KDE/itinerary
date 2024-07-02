/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "applicationcontroller.h"
#include "importcontroller.h"
#include "reservationmanager.h"
#include "transfermanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"
#include "tripgroupmodel.h"

#include <QAbstractItemModelTester>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

using namespace Qt::Literals;

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
        model.setTripGroupManager(&tgMgr);

        ImportController importer;
        importer.setReservationManager(&resMgr);

        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(insertSpy.size(), 1);
        auto idx = model.index(0, 0);
        QCOMPARE(idx.data(Qt::DisplayRole).toString(), "San Francisco Airport (March 2017)"_L1);

        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(insertSpy.size(), 2);
        QCOMPARE(insertSpy.back().at(1).toInt(), 1);
        idx = model.index(1, 0);
        QCOMPARE(idx.data(Qt::DisplayRole).toString(), "Randa (September 2017)"_L1);
        const auto tgId = idx.data(TripGroupModel::TripGroupIdRole).toString();
        QVERIFY(!tgId.isEmpty());

        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/multi-traveler-merge-with-countryinfo.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(model.rowCount(), 3);
        QCOMPARE(insertSpy.size(), 3);
        QCOMPARE(insertSpy.back().at(1).toInt(), 0);
        idx = model.index(0, 0);
        QCOMPARE(idx.data(Qt::DisplayRole).toString(), "Peretola (January 2000)"_L1);

        insertSpy.clear();

        auto tg = tgMgr.tripGroup(tgId);
        tg.setName(u"KDE Randa Meeting 2017"_s);
        tg.setNameIsAutomatic(false);
        tgMgr.updateTripGroup(tgId, tg);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(updateSpy.back().at(0).toModelIndex().row(), 2);
        idx = model.index(2, 0);
        QCOMPARE(idx.data(Qt::DisplayRole).toString(), "KDE Randa Meeting 2017"_L1);

        updateSpy.clear();

        tgMgr.removeReservationsInGroup(tgId);
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(removeSpy.size(), 1);
        QCOMPARE(removeSpy.back().at(1).toInt(), 2);

        QCOMPARE(insertSpy.size(), 0);
        QEXPECT_FAIL("", "missing regroup suspension during group removal", Continue);
        QCOMPARE(updateSpy.size(), 0);
    }
};
QTEST_GUILESS_MAIN(TripGroupModelTest)

#include "tripgroupmodeltest.moc"
