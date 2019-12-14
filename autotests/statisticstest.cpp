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

#include <statisticsmodel.h>
#include <statisticstimerangemodel.h>
#include <reservationmanager.h>

#include <QAbstractItemModelTester>
#include <QUrl>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

class StatisticsTest : public QObject
{
    Q_OBJECT
private:
    void clearReservations(ReservationManager *mgr)
    {
        const auto batches = mgr->batches(); // copy, as this is getting modified in the process
        for (const auto &id : batches) {
            mgr->removeBatch(id);
        }
        QCOMPARE(mgr->batches().size(), 0);
    }

    QByteArray readFile(const QString &fn)
    {
        QFile f(fn);
        f.open(QFile::ReadOnly);
        return f.readAll();
    }

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
        clearReservations(&resMgr);

        StatisticsModel stats;
        QSignalSpy changeSpy(&stats, &StatisticsModel::changed);
        stats.setReservationManager(&resMgr);

        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/akademy2017.json")));
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/akademy2018-program.json")));

        stats.setTimeRange({}, {});
        QVERIFY(!changeSpy.isEmpty());
        auto item = stats.totalCount();
        QCOMPARE(item.m_value, QLatin1String("13"));
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
        QCOMPARE(item.m_value, QLatin1String("1"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUnknown);

        changeSpy.clear();
        stats.setTimeRange({2017, 9, 1}, {2018, 1, 1});
        QVERIFY(!changeSpy.isEmpty());
        item = stats.totalCount();
        QCOMPARE(item.m_value, QLatin1String("8"));
        QCOMPARE(item.m_trend, StatisticsItem::TrendUp);
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
        clearReservations(&resMgr);
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/akademy2017.json")));
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/akademy2018-program.json")));

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
