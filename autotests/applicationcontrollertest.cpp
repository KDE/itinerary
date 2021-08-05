/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include <applicationcontroller.h>
#include <pkpassmanager.h>
#include <reservationmanager.h>
#include <documentmanager.h>
#include <favoritelocationmodel.h>
#include <healthcertificatemanager.h>
#include <transfermanager.h>
#include <tripgroupmanager.h>

#include <KItinerary/File>

#include <QUrl>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryFile>

class AppControllerTest : public QObject
{
    Q_OBJECT
private:
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
    }

    void testImportData()
    {
        PkPassManager passMgr;
        Test::clearAll(&passMgr);
        QSignalSpy passSpy(&passMgr, &PkPassManager::passAdded);
        QVERIFY(passSpy.isValid());

        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        QSignalSpy resSpy(&resMgr, &ReservationManager::reservationAdded);
        QVERIFY(resSpy.isValid());

        ApplicationController appController;
        QSignalSpy infoSpy(&appController, &ApplicationController::infoMessage);
        appController.setPkPassManager(&passMgr);
        appController.setReservationManager(&resMgr);

        appController.importData(readFile(QLatin1String(SOURCE_DIR "/data/4U8465-v1.json")));
        QCOMPARE(resSpy.size(), 1);
        QCOMPARE(passSpy.size(), 0);
        QCOMPARE(infoSpy.size(), 1);
        appController.importData(readFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        QCOMPARE(resSpy.size(), 2);
        QCOMPARE(passSpy.size(), 1);
        QCOMPARE(infoSpy.size(), 2);
        appController.importData("M1DOE/JOHN            EXXX007 TXLBRUSN 2592 110Y");
        QCOMPARE(resSpy.size(), 3);
        QCOMPARE(passSpy.size(), 1);
        QCOMPARE(infoSpy.size(), 3);
        // TODO PDF
    }

    void testImportFile()
    {
        PkPassManager passMgr;
        Test::clearAll(&passMgr);
        QSignalSpy passSpy(&passMgr, &PkPassManager::passAdded);
        QVERIFY(passSpy.isValid());

        ReservationManager resMgr;
        Test::clearAll(&resMgr);
        QSignalSpy resSpy(&resMgr, &ReservationManager::reservationAdded);
        QVERIFY(resSpy.isValid());

        ApplicationController appController;
        QSignalSpy infoSpy(&appController, &ApplicationController::infoMessage);
        appController.setPkPassManager(&passMgr);
        appController.setReservationManager(&resMgr);

        appController.importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/4U8465-v1.json")));
        QCOMPARE(resSpy.size(), 1);
        QCOMPARE(passSpy.size(), 0);
        QCOMPARE(infoSpy.size(), 1);
        appController.importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        QCOMPARE(resSpy.size(), 2);
        QCOMPARE(passSpy.size(), 1);
        QCOMPARE(infoSpy.size(), 2);
        // TODO PDF
    }

    void testExportFile()
    {
        PkPassManager passMgr;
        Test::clearAll(&passMgr);

        ReservationManager resMgr;
        Test::clearAll(&resMgr);

        DocumentManager docMgr;
        Test::clearAll(&docMgr);

        TripGroupManager tripGroupMgr;
        tripGroupMgr.setReservationManager(&resMgr);
        TransferManager transferMgr;
        transferMgr.setReservationManager(&resMgr);
        transferMgr.setTripGroupManager(&tripGroupMgr);

        FavoriteLocationModel favLoc;

        HealthCertificateManager healthCertMgr;

        ApplicationController appController;
        QSignalSpy infoSpy(&appController, &ApplicationController::infoMessage);
        appController.setPkPassManager(&passMgr);
        appController.setReservationManager(&resMgr);
        appController.setDocumentManager(&docMgr);
        appController.setTransferManager(&transferMgr);
        appController.setFavoriteLocationModel(&favLoc);
        appController.setHealthCertificateManager(&healthCertMgr);

        appController.importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/4U8465-v1.json")));
        appController.importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        QCOMPARE(resMgr.batches().size(), 2);
        QCOMPARE(passMgr.passes().size(), 1);
        QCOMPARE(infoSpy.size(), 2);

        QTemporaryFile tmp;
        QVERIFY(tmp.open());
        tmp.close();
        appController.exportToFile(QUrl::fromLocalFile(tmp.fileName()));
        QCOMPARE(infoSpy.size(), 3);

        KItinerary::File f(tmp.fileName());
        QVERIFY(f.open(KItinerary::File::Read));
        QCOMPARE(f.reservations().size(), 2);
        QCOMPARE(f.passes().size(), 1);

        Test::clearAll(&passMgr);
        Test::clearAll(&resMgr);
        Test::clearAll(&docMgr);
        QCOMPARE(resMgr.batches().size(), 0);
        QCOMPARE(passMgr.passes().size(), 0);

        appController.importFromUrl(QUrl::fromLocalFile(tmp.fileName()));
        QCOMPARE(resMgr.batches().size(), 2);
        QCOMPARE(passMgr.passes().size(), 1);
        QCOMPARE(infoSpy.size(), 4);

        Test::clearAll(&passMgr);
        Test::clearAll(&resMgr);
        QCOMPARE(resMgr.batches().size(), 0);
        QCOMPARE(passMgr.passes().size(), 0);

        QFile bundle(tmp.fileName());
        QVERIFY(bundle.open(QFile::ReadOnly));
        appController.importData(bundle.readAll());
        QCOMPARE(resMgr.batches().size(), 2);
        QCOMPARE(passMgr.passes().size(), 1);
        QCOMPARE(infoSpy.size(), 5);
    }
};

QTEST_GUILESS_MAIN(AppControllerTest)

#include "applicationcontrollertest.moc"
