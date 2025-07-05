/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "importcontroller.h"
#include "matrixsynccontent.h"
#include "matrixsyncstateevent.h"
#include "reservationmanager.h"

#include <Quotient/events/stateevent.h>

#include <QStandardPaths>
#include <QtTest/qtest.h>

class MatrixSyncContentTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testReservation()
    {
        ReservationManager mgr;
        Test::clearAll(&mgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&mgr);

        QCOMPARE(mgr.batches().size(), 0);
        ImportController importer;
        importer.setReservationManager(&mgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(mgr.batches().size(), 2);

        const auto batchId = mgr.batches()[0];
        QCOMPARE(mgr.reservationsForBatch(batchId).size(), 2);
        auto ev = MatrixSyncContent::stateEventForBatch(batchId, &mgr);
        QCOMPARE(ev.type(), "org.kde.itinerary.reservation"_L1);
        QCOMPARE(ev.stateKey(), batchId);
        QVERIFY(!ev.content().isEmpty());

        QCOMPARE(MatrixSyncContent::readBatch(ev, &mgr), batchId);
        QCOMPARE(mgr.batches().size(), 2);
        QCOMPARE(mgr.hasBatch(batchId), true);
        QCOMPARE(mgr.reservationsForBatch(batchId).size(), 2);

        mgr.removeBatch(batchId);
        QCOMPARE(mgr.batches().size(), 1);
        QCOMPARE(mgr.hasBatch(batchId), false);

        QCOMPARE(MatrixSyncContent::readBatch(ev, &mgr), batchId);
        QCOMPARE(mgr.batches().size(), 2);
        QCOMPARE(mgr.hasBatch(batchId), true);
        QCOMPARE(mgr.reservationsForBatch(batchId).size(), 2);

        ev = MatrixSyncContent::stateEventForDeletedBatch(batchId);
        QCOMPARE(ev.type(), "org.kde.itinerary.reservation"_L1);
        QCOMPARE(ev.stateKey(), batchId);
        QVERIFY(ev.content().isEmpty());

        QCOMPARE(MatrixSyncContent::readBatch(ev, &mgr), QString());
        QCOMPARE(mgr.batches().size(), 1);
        QCOMPARE(mgr.hasBatch(batchId), false);
    }
};

QTEST_GUILESS_MAIN(MatrixSyncContentTest)

#include "matrixsynccontenttest.moc"
