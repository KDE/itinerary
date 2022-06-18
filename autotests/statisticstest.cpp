/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include <statisticsmodel.h>
#include <statisticstimerangemodel.h>
#include <applicationcontroller.h>
#include <reservationmanager.h>
#include <tripgroupmanager.h>

#include <QAbstractItemModelTester>
#include <QUrl>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

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
        Test::clearAll(&resMgr);
        TripGroupManager::clear();
        TripGroupManager tgMgr;
        tgMgr.setReservationManager(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);

        StatisticsModel stats;
        QSignalSpy changeSpy(&stats, &StatisticsModel::changed);
        stats.setReservationManager(&resMgr);
        stats.setTripGroupManager(&tgMgr);

        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/../tests/akademy2017.json")));
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/../tests/akademy2018-program.json")));
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/timeline/flight-cancelation.json"))); // canceled flight, should not change stats

        stats.setTimeRange({}, {});
        QVERIFY(!changeSpy.isEmpty());
        auto item = stats.totalCount();
        QCOMPARE(item.m_value, QLatin1String("2"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        item = stats.totalNights();
        QCOMPARE(item.m_value, QLatin1String("13"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        item = stats.totalDistance();
        QCOMPARE(item.m_value, QLatin1String("6,182 km"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        item = stats.totalCO2();
        QCOMPARE(item.m_value, QLatin1String("1,673 kg"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);

        item = stats.flightCount();
        QCOMPARE(item.m_value, QLatin1String("6"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        item = stats.flightDistance();
        QCOMPARE(item.m_value, QLatin1String("5,859 km"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        item = stats.flightCO2();
        QCOMPARE(item.m_value, QLatin1String("1,668 kg"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);

        item = stats.trainCount();
        QCOMPARE(item.m_value, QLatin1String("4"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        item = stats.trainDistance();
        QCOMPARE(item.m_value, QLatin1String("323 km"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);
        item = stats.trainCO2();
        QCOMPARE(item.m_value, QLatin1String("4.5 kg"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);

        item = stats.carCount();
        QCOMPARE(item.m_value, QLatin1String("0"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);

        changeSpy.clear();
        stats.setTimeRange({2017, 9, 1}, {2018, 1, 1});
        QVERIFY(!changeSpy.isEmpty());
        item = stats.totalCount();
        QCOMPARE(item.m_value, QLatin1String("1"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnchanged);
        item = stats.totalNights();
        QCOMPARE(item.m_value, QLatin1String("5"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendDown);
        item = stats.totalDistance();
        QCOMPARE(item.m_value, QLatin1String("1,642 km"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendDown);
        item = stats.totalCO2();
        QCOMPARE(item.m_value, QLatin1String("380 kg"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendDown);

        item = stats.flightCount();
        QCOMPARE(item.m_value, QLatin1String("2"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendDown);
        item = stats.flightDistance();
        QCOMPARE(item.m_value, QLatin1String("1,319 km"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendDown);
        item = stats.flightCO2();
        QCOMPARE(item.m_value, QLatin1String("375 kg"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendDown);

        item = stats.trainCount();
        QCOMPARE(item.m_value, QLatin1String("4"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUp);
        item = stats.trainDistance();
        QCOMPARE(item.m_value, QLatin1String("323 km"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUp);
        item = stats.trainCO2();
        QCOMPARE(item.m_value, QLatin1String("4.5 kg"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUp);
    }

    void testTimeRangeModel()
    {
        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);

        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/../tests/akademy2017.json")));
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/../tests/akademy2018-program.json")));

        StatisticsTimeRangeModel model;
        QAbstractItemModelTester tester(&model);
        QCOMPARE(model.rowCount(), 1);

        model.setReservationManager(&resMgr);
        QCOMPARE(model.rowCount(), 3);
        QCOMPARE(model.data(model.index(1, 0), Qt::DisplayRole).toString(), QLatin1String("2018"));
        QCOMPARE(model.data(model.index(2, 0), Qt::DisplayRole).toString(), QLatin1String("2017"));

        QCOMPARE(model.data(model.index(1, 0), StatisticsTimeRangeModel::BeginRole).toDate(), QDate(2018, 1, 1));
        QCOMPARE(model.data(model.index(1, 0), StatisticsTimeRangeModel::EndRole).toDate(), QDate(2018, 12, 31));
        QCOMPARE(model.data(model.index(2, 0), StatisticsTimeRangeModel::BeginRole).toDate(), QDate());
        QCOMPARE(model.data(model.index(2, 0), StatisticsTimeRangeModel::EndRole).toDate(), QDate(2017, 12, 31));
    }
};

QTEST_GUILESS_MAIN(StatisticsTest)

#include "statisticstest.moc"
