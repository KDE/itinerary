/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "importcontroller.h"
#include "reservationmanager.h"
#include "tickettokenmodel.h"

#include <KItinerary/Reservation>

#include <QAbstractItemModelTester>
#include <QStandardPaths>
#include <QtTest/qtest.h>

using namespace Qt::Literals;

class TicketTokenModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testMultiTraveler()
    {
        ReservationManager mgr;
        Test::clearAll(&mgr);

        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&mgr);
        ImportController importer;
        importer.setReservationManager(&mgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(mgr.batches().size(), 2);

        TicketTokenModel model;
        QAbstractItemModelTester modelTest(&model);
        QCOMPARE(model.rowCount(), 0);

        const auto resIds = mgr.reservationsForBatch(mgr.batches()[0]);
        QCOMPARE(resIds.size(), 2);
        model.setBatchId(mgr.batches()[0]);
        model.setReservationManager(&mgr);
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.initialIndex(), 0);
        QVERIFY(model.data(model.index(0, 0), Qt::DisplayRole).toString().contains("Eva Green"_L1));
        QVERIFY(model.data(model.index(1, 0), Qt::DisplayRole).toString().contains("John Green"_L1));

        auto res = mgr.reservation(resIds[1]).value<KItinerary::FlightReservation>();
        auto person = res.underName().value<KItinerary::Person>();
        person.setName(u"Changed Name"_s);
        res.setUnderName(person);
        mgr.updateReservation(resIds[1], res);
        QVERIFY(model.data(model.index(0, 0), Qt::DisplayRole).toString().contains("Eva Green"_L1));
        QVERIFY(model.data(model.index(1, 0), Qt::DisplayRole).toString().contains("Changed Name"_L1));
    }

    void testMultiTicket()
    {
        ReservationManager mgr;
        Test::clearAll(&mgr);

        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&mgr);
        ImportController importer;
        importer.setReservationManager(&mgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/multi-ticket.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(mgr.batches().size(), 1);

        TicketTokenModel model;
        QAbstractItemModelTester modelTest(&model);
        QCOMPARE(model.rowCount(), 0);

        const auto resIds = mgr.reservationsForBatch(mgr.batches()[0]);
        QCOMPARE(resIds.size(), 2);
        model.setReservationManager(&mgr);
        model.setBatchId(mgr.batches()[0]);
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.initialIndex(), 0);

        mgr.removeReservation(resIds[1]);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.initialIndex(), 0);
    }

    void testSingle()
    {
        ReservationManager mgr;
        Test::clearAll(&mgr);

        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&mgr);
        ImportController importer;
        importer.setReservationManager(&mgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/multi-ticket.json")));
        ctrl->commitImport(&importer);
        QCOMPARE(mgr.batches().size(), 1);

        TicketTokenModel model;
        QAbstractItemModelTester modelTest(&model);
        QCOMPARE(model.rowCount(), 0);

        const auto resIds = mgr.reservationsForBatch(mgr.batches()[0]);
        mgr.removeReservation(resIds[1]);
        model.setReservationManager(&mgr);
        model.setBatchId(mgr.batches()[0]);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.initialIndex(), 0);
    }
};

QTEST_GUILESS_MAIN(TicketTokenModelTest)

#include "tickettokenmodeltest.moc"
