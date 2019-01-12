/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

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

#include <reservationmanager.h>
#include <tripgroup.h>
#include <tripgroupmanager.h>

#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

class TripGroupTest : public QObject
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
        qputenv("LC_ALL", "en_US.utf-8");
        QStandardPaths::setTestModeEnabled(true);
    }

    void init()
    {
        TripGroupManager::clear();
    }

    void testGrouping()
    {
        ReservationManager resMgr;
        clearReservations(&resMgr);
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        {
            TripGroupManager mgr;
            QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
            QVERIFY(addSpy.isValid());
            mgr.setReservationManager(&resMgr);
            QCOMPARE(addSpy.size(), 1);
            auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
            QCOMPARE(g.elements().size(), resMgr.batches().size());
        }

        TripGroupManager::clear();
        clearReservations(&resMgr);
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));
        {
            TripGroupManager mgr;
            QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
            QVERIFY(addSpy.isValid());
            mgr.setReservationManager(&resMgr);
            QCOMPARE(addSpy.size(), 1);
            auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
            QCOMPARE(g.elements().size(), resMgr.batches().size());
        }

        TripGroupManager::clear();
        clearReservations(&resMgr);
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/timeline/multi-traveler-merge-with-countryinfo.json")));
        {
            TripGroupManager mgr;
            QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
            QVERIFY(addSpy.isValid());
            mgr.setReservationManager(&resMgr);
            QCOMPARE(addSpy.size(), 1);
            auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
            QCOMPARE(g.elements().size(), resMgr.batches().size());
        }

        TripGroupManager::clear();
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));
        {
            TripGroupManager mgr;
            QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
            QVERIFY(addSpy.isValid());
            mgr.setReservationManager (&resMgr);
            QCOMPARE(addSpy.size(), 3);
            QCOMPARE(mgr.tripGroup(addSpy.at(0).at(0).toString()).elements().size(), 2);
            QCOMPARE(mgr.tripGroup(addSpy.at(1).at(0).toString()).elements().size(), 2);
            QCOMPARE(mgr.tripGroup(addSpy.at(2).at(0).toString()).elements().size(), 12);
        }
    }

    void testChanges()
    {
        ReservationManager resMgr;
        clearReservations(&resMgr);
        TripGroupManager mgr;
        mgr.setReservationManager(&resMgr);

        QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
        QSignalSpy changeSpy(&mgr, &TripGroupManager::tripGroupChanged);
        QSignalSpy rmSpy(&mgr, &TripGroupManager::tripGroupRemoved);

        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        QCOMPARE(addSpy.size(), 1);
        auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
        QCOMPARE(g.elements().size(), resMgr.batches().size());
        QCOMPARE(changeSpy.size(), 0);

        changeSpy.clear();
        clearReservations(&resMgr);
        QCOMPARE(rmSpy.size(), 1);
        QCOMPARE(changeSpy.size(), 0);
    }

    void testGroupName_data()
    {
        QTest::addColumn<QString>("fileName");
        QTest::addColumn<QString>("expectedName");

        QTest::newRow("SFO one way") << QStringLiteral(SOURCE_DIR "/data/google-multi-passenger-flight.json") << QStringLiteral("San Francisco Airport (March 2017)");
        QTest::newRow("FLR one way multi traveller") << QStringLiteral(SOURCE_DIR "/data/timeline/multi-traveler-merge-with-countryinfo.json") << QStringLiteral("Peretola (January 2000)");
        QTest::newRow("Randa") << QStringLiteral(SOURCE_DIR "/../tests/randa2017.json") << QStringLiteral("Randa (September 2017)");
        QTest::newRow("Almeria") << QStringLiteral(SOURCE_DIR "/../tests/akademy2017.json") << QStringLiteral("Almería (July 2017)");
        QTest::newRow("Symmetric") << QStringLiteral(SOURCE_DIR "/data/tripgroup/deutschebahn_two-leg-return.txt.json") << QStringLiteral("Somewhere(Specific) (November 2027)");
        QTest::newRow("Symmetric, 2 elements") << QStringLiteral(SOURCE_DIR "/data/tripgroup/flight-direct-return.json") << QStringLiteral("Oslo Airport (June 2018)");
        QTest::newRow("Triangular, different PNR") << QStringLiteral(SOURCE_DIR "/data/tripgroup/train-triangular-different-pnr.json") << QStringLiteral("Somewhere (February/March 2018)");
    }

    void testGroupName()
    {
        QFETCH(QString, fileName);
        QFETCH(QString, expectedName);

        ReservationManager resMgr;
        clearReservations(&resMgr);
        resMgr.importReservation(readFile(fileName));
        TripGroupManager mgr;
        QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
        mgr.setReservationManager(&resMgr);
        QCOMPARE(addSpy.size(), 1);
        auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
        QCOMPARE(g.elements().size(), resMgr.batches().size());
        QCOMPARE(g.name(), expectedName);
    }

    void testLeadingAppendixRemoval()
    {
        ReservationManager resMgr;
        clearReservations(&resMgr);
        resMgr.importReservation(readFile(QStringLiteral(SOURCE_DIR "/data/tripgroup/leading-appendix.json")));
        TripGroupManager mgr;
        QSignalSpy addSpy(&mgr, &TripGroupManager::tripGroupAdded);
        mgr.setReservationManager(&resMgr);
        QCOMPARE(addSpy.size(), 1);
        auto g = mgr.tripGroup(addSpy.at(0).at(0).toString());
        QCOMPARE(g.elements().size(), resMgr.batches().size() - 1);
        QCOMPARE(g.name(), QStringLiteral("Oslo Airport (June 2000)"));
    }
};
QTEST_GUILESS_MAIN(TripGroupTest)

#include "tripgrouptest.moc"
