/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "applicationcontroller.h"
#include "importcontroller.h"
#include "reservationmanager.h"
#include "statisticsmodel.h"
#include "statisticstimerangemodel.h"
#include "transfermanager.h"
#include "tripgroupmanager.h"

#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QUrl>
#include <QtTest/qtest.h>

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("LANG", "C");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class StatisticsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
        qunsetenv("LANG");
        qunsetenv("LANGUAGE");
        qunsetenv("LC_CTYPE");
        QStandardPaths::setTestModeEnabled(true);
    }

    void testStats()
    {
        ReservationManager resMgr;
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        TripGroupManager::clear();
        TripGroupManager tgMgr;
        tgMgr.setReservationManager(&resMgr);
        tgMgr.setTransferManager(&transferMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ctrl->setTransferManager(&transferMgr);

        StatisticsModel stats;
        QSignalSpy changeSpy(&stats, &StatisticsModel::changed);
        stats.setReservationManager(&resMgr);
        stats.setTripGroupManager(&tgMgr);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/akademy2017.json")));
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/akademy2018-program.json")));
        ctrl->commitImport(&importer);
        importer.importFromUrl(
            QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/flight-cancelation.json"))); // canceled flight, should not change stats
        ctrl->commitImport(&importer);

        stats.setTimeRange({}, {});
        QVERIFY(!changeSpy.isEmpty());
        auto item = stats.totalCount();
        QCOMPARE(item.m_value, QLatin1StringView("2"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        QCOMPARE(item.m_hasData, true);
        item = stats.totalNights();
        QCOMPARE(item.m_value, QLatin1StringView("13"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        QCOMPARE(item.m_hasData, true);
        item = stats.totalDistance();
        QCOMPARE(item.m_value, QLatin1StringView("6,182 km"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        QCOMPARE(item.m_hasData, true);
        item = stats.totalCO2();
        QCOMPARE(item.m_value, QLatin1StringView("1,674 kg"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        QCOMPARE(item.m_hasData, true);

        item = stats.flightCount();
        QCOMPARE(item.m_value, QLatin1StringView("6"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        QCOMPARE(item.m_hasData, true);
        item = stats.flightDistance();
        QCOMPARE(item.m_value, QLatin1StringView("5,859 km"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        QCOMPARE(item.m_hasData, true);
        item = stats.flightCO2();
        QCOMPARE(item.m_value, QLatin1StringView("1,670 kg"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        QCOMPARE(item.m_hasData, true);

        item = stats.trainCount();
        QCOMPARE(item.m_value, QLatin1StringView("4"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        QCOMPARE(item.m_hasData, true);
        item = stats.trainDistance();
        QCOMPARE(item.m_value, QLatin1StringView("323 km"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        QCOMPARE(item.m_hasData, true);
        item = stats.trainCO2();
        QCOMPARE(item.m_value, QLatin1StringView("4.5 kg"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        QCOMPARE(item.m_hasData, true);

        item = stats.carCount();
        QCOMPARE(item.m_value, QLatin1StringView("0"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        QCOMPARE(item.m_hasData, false);

        changeSpy.clear();
        stats.setTimeRange({2017, 9, 1}, {2018, 1, 1});
        QVERIFY(!changeSpy.isEmpty());
        item = stats.totalCount();
        QCOMPARE(item.m_value, QLatin1StringView("1"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnchanged);
        QCOMPARE(item.m_hasData, true);
        item = stats.totalNights();
        QCOMPARE(item.m_value, QLatin1StringView("5"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendDown);
        QCOMPARE(item.m_hasData, true);
        item = stats.totalDistance();
        QCOMPARE(item.m_value, QLatin1StringView("1,642 km"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendDown);
        QCOMPARE(item.m_hasData, true);
        item = stats.totalCO2();
        QCOMPARE(item.m_value, QLatin1StringView("381 kg"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendDown);
        QCOMPARE(item.m_hasData, true);

        item = stats.flightCount();
        QCOMPARE(item.m_value, QLatin1StringView("2"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendDown);
        QCOMPARE(item.m_hasData, true);
        item = stats.flightDistance();
        QCOMPARE(item.m_value, QLatin1StringView("1,319 km"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendDown);
        QCOMPARE(item.m_hasData, true);
        item = stats.flightCO2();
        QCOMPARE(item.m_value, QLatin1StringView("376 kg"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendDown);
        QCOMPARE(item.m_hasData, true);

        item = stats.trainCount();
        QCOMPARE(item.m_value, QLatin1StringView("4"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUp);
        QCOMPARE(item.m_hasData, true);
        item = stats.trainDistance();
        QCOMPARE(item.m_value, QLatin1StringView("323 km"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUp);
        QCOMPARE(item.m_hasData, true);
        item = stats.trainCO2();
        QCOMPARE(item.m_value, QLatin1StringView("4.5 kg"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUp);
        QCOMPARE(item.m_hasData, true);

        QCOMPARE(stats.boatCount().m_value, QLatin1StringView("0"));
        QCOMPARE(stats.boatCount().m_hasData, false);
        QCOMPARE(stats.boatDistance().m_hasData, false);
        QCOMPARE(stats.boatCO2().m_hasData, false);
    }

    void testTimeRangeModel()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);

        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/akademy2017.json")));
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/akademy2018-program.json")));
        ctrl->commitImport(&importer);

        StatisticsTimeRangeModel model;
        QAbstractItemModelTester tester(&model);
        QCOMPARE(model.rowCount(), 1);

        model.setReservationManager(&resMgr);
        QCOMPARE(model.rowCount(), 3);
        QCOMPARE(model.data(model.index(1, 0), Qt::DisplayRole).toString(), QLatin1StringView("2018"));
        QCOMPARE(model.data(model.index(2, 0), Qt::DisplayRole).toString(), QLatin1StringView("2017"));

        QCOMPARE(model.data(model.index(1, 0), StatisticsTimeRangeModel::BeginRole).toDate(), QDate(2018, 1, 1));
        QCOMPARE(model.data(model.index(1, 0), StatisticsTimeRangeModel::EndRole).toDate(), QDate(2018, 12, 31));
        QCOMPARE(model.data(model.index(2, 0), StatisticsTimeRangeModel::BeginRole).toDate(), QDate());
        QCOMPARE(model.data(model.index(2, 0), StatisticsTimeRangeModel::EndRole).toDate(), QDate(2017, 12, 31));
    }
};

QTEST_GUILESS_MAIN(StatisticsTest)

#include "statisticstest.moc"
