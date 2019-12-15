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

        TransferManager::clear();
        TransferManager mgr;
        mgr.overrideCurrentDateTime(QDateTime({2017, 1, 1}, {}));
        mgr.setReservationManager(&resMgr);
        mgr.setTripGroupManager(&tgMgr);
        QSignalSpy addSpy(&mgr, &TransferManager::transferAdded);
        QSignalSpy changeSpy(&mgr, &TransferManager::transferChanged);
        QSignalSpy removeSpy(&mgr, &TransferManager::transferRemoved);

        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));
//         resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/akademy2017.json")));
//         resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/akademy2018-program.json")));

        QCOMPARE(addSpy.size(), 3); // to/from home, and one inbetween

        auto batchId = resMgr.batches().at(0);
        auto transfer = mgr.transfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::Pending);
        QCOMPARE(transfer.anchorTime(), QDateTime({2017, 9, 10}, {6, 45}, QTimeZone("Europe/Berlin")));
        QCOMPARE(transfer.alignment(), Transfer::Before);
        QCOMPARE(transfer.reservationId(), batchId);

        transfer = mgr.transfer(batchId, Transfer::After);
        QCOMPARE(transfer.state(), Transfer::UndefinedState);

        // verify persistence
        TransferManager mgr2;
        transfer = mgr2.transfer(batchId, Transfer::Before);
        QCOMPARE(transfer.state(), Transfer::Pending);
        QCOMPARE(transfer.anchorTime(), QDateTime({2017, 9, 10}, {6, 45}, QTimeZone("Europe/Berlin")));
        QCOMPARE(transfer.alignment(), Transfer::Before);
        QCOMPARE(transfer.reservationId(), batchId);
    }
};

QTEST_GUILESS_MAIN(TransferTest)

#include "transfertest.moc"
