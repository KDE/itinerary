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
#include <pkpassmanager.h>

#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>
#include <KItinerary/Place>
#include <KItinerary/Visit>

class ReservationManagerTest : public QObject
{
    Q_OBJECT
private:
    void clearReservations(ReservationManager *mgr)
    {
        for (const auto &id : mgr->reservations()) {
            mgr->removeReservation(id);
        }
    }

    void clearPasses(PkPassManager *mgr)
    {
        for (const auto &id : mgr->passes()) {
            mgr->removePass(id);
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
        QStandardPaths::setTestModeEnabled(true);
    }

    void testOperations()
    {
        ReservationManager mgr;
        clearReservations(&mgr);

        QSignalSpy addSpy(&mgr, &ReservationManager::reservationAdded);
        QVERIFY(addSpy.isValid());
        QSignalSpy updateSpy(&mgr, &ReservationManager::reservationUpdated);
        QVERIFY(updateSpy.isValid());
        QSignalSpy rmSpy(&mgr, &ReservationManager::reservationRemoved);
        QVERIFY(rmSpy.isValid());

        QVERIFY(mgr.reservations().isEmpty());
        mgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/4U8465-v1.json")));

        auto res = mgr.reservations();
        QCOMPARE(res.size(), 1);
        const auto &resId = res.at(0);
        QVERIFY(!resId.isEmpty());

        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(addSpy.at(0).at(0).toString(), resId);
        QVERIFY(updateSpy.isEmpty());
        QVERIFY(!mgr.reservation(resId).isNull());

        mgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/4U8465-v2.json")));
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(mgr.reservations().size(), 1);
        QCOMPARE(updateSpy.at(0).at(0).toString(), resId);
        QVERIFY(mgr.reservation(resId).isValid());

        mgr.removeReservation(resId);
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(rmSpy.size(), 1);
        QCOMPARE(rmSpy.at(0).at(0).toString(), resId);
        QVERIFY(mgr.reservations().isEmpty());
        QVERIFY(mgr.reservation(resId).isNull());

        clearReservations(&mgr);
        auto attraction = KItinerary::TouristAttraction();
        attraction.setName(QStringLiteral("Sky Tree"));
        auto visit = KItinerary::TouristAttractionVisit();
        visit.setTouristAttraction(attraction);

        mgr.addReservation(QVariant::fromValue(visit));
        auto addedResId = mgr.reservations().at(0);

        QCOMPARE(addSpy.size(), 2);
        QVERIFY(!mgr.reservations().isEmpty());
        QCOMPARE(mgr.reservations().size(), 1);
        QCOMPARE(addSpy.at(1).at(0).toString(), addedResId);
        QVERIFY(mgr.reservation(addedResId).isValid());
    }

    void testPkPassChanges()
    {
        PkPassManager passMgr;
        clearPasses(&passMgr);

        ReservationManager mgr;
        mgr.setPkPassManager(&passMgr);
        clearReservations(&mgr);

        QSignalSpy addSpy(&mgr, &ReservationManager::reservationAdded);
        QVERIFY(addSpy.isValid());
        QSignalSpy updateSpy(&mgr, &ReservationManager::reservationUpdated);
        QVERIFY(updateSpy.isValid());

        QVERIFY(mgr.reservations().isEmpty());
        const auto passId = QStringLiteral("pass.booking.kde.org/MTIzNA==");

        passMgr.importPass(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        QCOMPARE(addSpy.size(), 1);
        QVERIFY(updateSpy.isEmpty());

        passMgr.importPass(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v2.pkpass")));
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
    }
};
QTEST_GUILESS_MAIN(ReservationManagerTest)

#include "reservationmanagertest.moc"
