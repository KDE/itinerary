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

#include <transfer.h>
#include <transfermanager.h>
#include <reservationmanager.h>
#include <tripgroupmanager.h>
#include <favoritelocationmodel.h>

#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTimeZone>

class TransferTest : public QObject
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
        QStandardPaths::setTestModeEnabled(true);

        qRegisterMetaType<Transfer::Alignment>();
    }

    void testTransferManager()
    {
        ReservationManager resMgr;
        clearReservations(&resMgr);

        TripGroupManager::clear();
        TripGroupManager tgMgr;
        tgMgr.setReservationManager(&resMgr);

        FavoriteLocationModel favLocModel;
        while (favLocModel.rowCount()) {
            favLocModel.removeLocation(0);
        }
        FavoriteLocation favLoc;
        favLoc.setLatitude(52.51860f);
        favLoc.setLongitude(13.37630f);
        favLoc.setName(QStringLiteral("name"));
        favLocModel.setFavoriteLocations({favLoc});
        QCOMPARE(favLocModel.rowCount(), 1);

        TransferManager::clear();
        TransferManager mgr;
        mgr.setFavoriteLocationModel(&favLocModel);
        mgr.overrideCurrentDateTime(QDateTime({2017, 1, 1}, {}));
        mgr.setReservationManager(&resMgr);
        mgr.setTripGroupManager(&tgMgr);
        QSignalSpy addSpy(&mgr, &TransferManager::transferAdded);
        QSignalSpy changeSpy(&mgr, &TransferManager::transferChanged);
        QSignalSpy removeSpy(&mgr, &TransferManager::transferRemoved);

        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));
//         resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/akademy2017.json")));
//         resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/akademy2018-program.json")));

        QCOMPARE(addSpy.size() - removeSpy.size(), 4); // to/from home, and one inbetween

        auto batchId = resMgr.batches().at(0);
        auto transfer = mgr.transfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::Pending);
        QCOMPARE(transfer.anchorTime(), QDateTime({2017, 9, 10}, {6, 45}, QTimeZone("Europe/Berlin")));
        QCOMPARE(transfer.anchorTimeDelta(), 3600);
        QCOMPARE(transfer.alignment(), Transfer::Before);
        QCOMPARE(transfer.reservationId(), batchId);
        QVERIFY(transfer.from().hasCoordinate());
        QCOMPARE(transfer.from().latitude(), 52.51860f);
        QCOMPARE(transfer.from().longitude(), 13.37630f);
        QVERIFY(transfer.to().hasCoordinate());
        QCOMPARE(transfer.to().name(), QLatin1String("Berlin Tegel"));
        QVERIFY(!mgr.canAddTransfer(batchId, Transfer::Before));

        transfer = mgr.transfer(batchId, Transfer::After);
        QCOMPARE(transfer.state(), Transfer::UndefinedState);
        QVERIFY(!mgr.canAddTransfer(batchId, Transfer::After));

        // verify persistence
        TransferManager mgr2;
        transfer = mgr2.transfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::Pending);
        QCOMPARE(transfer.anchorTime(), QDateTime({2017, 9, 10}, {6, 45}, QTimeZone("Europe/Berlin")));
        QCOMPARE(transfer.alignment(), Transfer::Before);
        QCOMPARE(transfer.reservationId(), batchId);
        QVERIFY(transfer.hasLocations());
        QVERIFY(transfer.from().hasCoordinate());
        QCOMPARE(transfer.from().latitude(), 52.51860f);
        QCOMPARE(transfer.from().longitude(), 13.37630f);
        QVERIFY(transfer.to().hasCoordinate());
        QCOMPARE(transfer.to().name(), QLatin1String("Berlin Tegel"));

        // operations
        addSpy.clear();
        changeSpy.clear();
        removeSpy.clear();

        KPublicTransport::Journey jny;
        KPublicTransport::JourneySection section;
        section.setScheduledDepartureTime(QDateTime({2017, 9, 10}, {5, 30}, QTimeZone("Europe/Berlin")));
        section.setScheduledArrivalTime(QDateTime({2017, 9, 10}, {6, 0}, QTimeZone("Europe/Berlin")));
        jny.setSections({section});
        mgr.setJourneyForTransfer(transfer, jny);
        QCOMPARE(addSpy.size(), 0);
        QCOMPARE(changeSpy.size(), 1);
        QCOMPARE(removeSpy.size(), 0);

        transfer = mgr.transfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::Selected);
        QCOMPARE(transfer.journey().sections().size(), 1);
        QCOMPARE(transfer.journey().scheduledArrivalTime(), QDateTime({2017, 9, 10}, {6, 0}, QTimeZone("Europe/Berlin")));
        QCOMPARE(transfer.anchorTimeDelta(), 45*60);

        addSpy.clear();
        changeSpy.clear();
        removeSpy.clear();

        mgr.discardTransfer(transfer);
        QCOMPARE(addSpy.size(), 0);
        QCOMPARE(changeSpy.size(), 0);
        QCOMPARE(removeSpy.size(), 1);
        transfer = mgr.transfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::Discarded);
        QVERIFY(mgr.canAddTransfer(batchId, Transfer::Before));

        transfer = mgr.addTransfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::Pending);
        QCOMPARE(transfer.anchorTime(), QDateTime({2017, 9, 10}, {6, 45}, QTimeZone("Europe/Berlin")));
        QCOMPARE(transfer.alignment(), Transfer::Before);
        QCOMPARE(transfer.reservationId(), batchId);
        QVERIFY(transfer.hasLocations());
        QVERIFY(!mgr.canAddTransfer(batchId, Transfer::Before));
    }
};

QTEST_GUILESS_MAIN(TransferTest)

#include "transfertest.moc"
