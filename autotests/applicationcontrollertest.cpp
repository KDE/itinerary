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

#include <applicationcontroller.h>
#include <pkpassmanager.h>
#include <reservationmanager.h>

#include <QUrl>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

class AppControllerTest : public QObject
{
    Q_OBJECT
private:
    void clearPasses(PkPassManager *mgr)
    {
        for (const auto id : mgr->passes())
            mgr->removePass(id);
    }

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
        qputenv("TZ", "UTC");
        QStandardPaths::setTestModeEnabled(true);
    }

    void testImportData()
    {
        PkPassManager passMgr;
        clearPasses(&passMgr);
        QSignalSpy passSpy(&passMgr, &PkPassManager::passAdded);
        QVERIFY(passSpy.isValid());

        ReservationManager resMgr;
        clearReservations(&resMgr);
        QSignalSpy resSpy(&resMgr, &ReservationManager::reservationAdded);
        QVERIFY(resSpy.isValid());

        ApplicationController appController;
        appController.setPkPassManager(&passMgr);
        appController.setReservationManager(&resMgr);

        appController.importData(readFile(QLatin1String(SOURCE_DIR "/data/4U8465-v1.json")));
        QCOMPARE(resSpy.size(), 1);
        QCOMPARE(passSpy.size(), 0);
        appController.importData(readFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        QCOMPARE(resSpy.size(), 1);
        QCOMPARE(passSpy.size(), 1);
        appController.importData("M1DOE/JOHN            EXXX007 TXLBRUSN 2592 110Y");
        QCOMPARE(resSpy.size(), 2);
        QCOMPARE(passSpy.size(), 1);
        // TODO PDF
    }

    void testImportFile()
    {
        PkPassManager passMgr;
        clearPasses(&passMgr);
        QSignalSpy passSpy(&passMgr, &PkPassManager::passAdded);
        QVERIFY(passSpy.isValid());

        ReservationManager resMgr;
        clearReservations(&resMgr);
        QSignalSpy resSpy(&resMgr, &ReservationManager::reservationAdded);
        QVERIFY(resSpy.isValid());

        ApplicationController appController;
        appController.setPkPassManager(&passMgr);
        appController.setReservationManager(&resMgr);

        appController.importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/4U8465-v1.json")));
        QCOMPARE(resSpy.size(), 1);
        QCOMPARE(passSpy.size(), 0);
        appController.importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        QCOMPARE(resSpy.size(), 1);
        QCOMPARE(passSpy.size(), 1);
        // TODO PDF
    }
};

QTEST_GUILESS_MAIN(AppControllerTest)

#include "applicationcontrollertest.moc"
