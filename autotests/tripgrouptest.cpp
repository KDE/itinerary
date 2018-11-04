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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
        for (const auto id : mgr->reservations()) {
            mgr->removeReservation(id);
        }
    }

    QByteArray readFile(const QString &fn)
    {
        QFile f(fn);
        f.open(QFile::ReadOnly);
        return f.readAll();
    }

private slots:
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
            QCOMPARE(g.elements().size(), resMgr.reservations().size());
            QCOMPARE(g.name(), QStringLiteral("San Francisco Airport (March 2017)"));
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
            QCOMPARE(g.elements().size(), resMgr.reservations().size());
            QCOMPARE(g.name(), QStringLiteral("Zermatt (September 2017)"));
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
            QCOMPARE(g.elements().size(), resMgr.reservations().size());
            QCOMPARE(g.name(), QStringLiteral("Peretola (January 2000)"));
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
            QCOMPARE(mgr.tripGroup(addSpy.at(0).at(0).toString()).elements().size(), 4);
            QCOMPARE(mgr.tripGroup(addSpy.at(1).at(0).toString()).elements().size(), 4);
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
        QCOMPARE(g.elements().size(), resMgr.reservations().size());
        QCOMPARE(changeSpy.size(), 1);

        changeSpy.clear();
        clearReservations(&resMgr);
        QEXPECT_FAIL("", "not implemented yet", Continue);
        QCOMPARE(rmSpy.size(), 1);
        QEXPECT_FAIL("", "not implemented yet", Continue);
        QCOMPARE(changeSpy.size(), 1);
    }
};
QTEST_GUILESS_MAIN(TripGroupTest)

#include "tripgrouptest.moc"
