/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "costaccumulator.h"
#include "importcontroller.h"
#include "reservationmanager.h"

#include <QtTest/qtest.h>

using namespace Qt::Literals;

class CostAccumulatorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testAccumulate_data()
    {
        QTest::addColumn<QString>("inFile");
        QTest::addColumn<double>("cost");
        QTest::addColumn<QString>("currency");
        QTest::addColumn<QString>("targetCurrency");

        QTest::newRow("single-res-num") << u"" SOURCE_DIR "/data/cost-accumulator/single-res-num.json"_s << 200.0 << u"CHF"_s << u"CHF"_s;
        QTest::newRow("mixed-currency-converting") << u"" SOURCE_DIR "/data/cost-accumulator/mixed-currency.json"_s << 46.759716 << u"EUR"_s << u"EUR"_s;
        QTest::newRow("mixed-currency-not-converting") << u"" SOURCE_DIR "/data/cost-accumulator/mixed-currency.json"_s << (double)NAN << QString() << QString();
        QTest::newRow("batch-no-resnum-same-price") << u"" SOURCE_DIR "/data/cost-accumulator/batch-no-resnum-same-price.json"_s << 23.0 << u"EUR"_s << u"EUR"_s;
        QTest::newRow("batch-no-resnum-different-price") << u"" SOURCE_DIR "/data/cost-accumulator/batch-no-resnum-different-prices.json"_s << 65.0 << u"EUR"_s << u"EUR"_s;
        QTest::newRow("batch-resnum-same-price") << u"" SOURCE_DIR "/data/cost-accumulator/batch-resnum-same-price.json"_s << 42.0 << u"EUR"_s << u"EUR"_s;
    }

    void testAccumulate()
    {
        QFETCH(QString, inFile);
        QFETCH(double, cost);
        QFETCH(QString, currency);
        QFETCH(QString, targetCurrency);

        ReservationManager resMgr;
        Test::clearAll(&resMgr);

        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(inFile));
        ctrl->commitImport(&importer);
        QVERIFY(!resMgr.batches().empty());

        CostAccumulator accu(&resMgr);
        accu.setTargetCurrency(targetCurrency);
        for (const auto &batchId : resMgr.batches()) {
            accu.addBatch(batchId);
        }

        const auto total = accu.totalCost();
        QCOMPARE(total.currency, currency);
        QCOMPARE(total.value, cost);
    }
};

QTEST_GUILESS_MAIN(CostAccumulatorTest)

#include "costaccumulatortest.moc"
