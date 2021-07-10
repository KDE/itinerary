/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
    void clearPasses(PkPassManager *mgr)
    {
        for (const auto &id : mgr->passes()) {
            mgr->removePass(id);
        }
    }

    void clearReservations(ReservationManager *mgr)
    {
        const auto batches = mgr->batches(); // copy, as this is getting modified in the process
        for (const auto &id : batches) {
            mgr->removeBatch(id);
        }
        QCOMPARE(mgr->batches().size(), 0);
    }

    void clearDocuments(DocumentManager *mgr)
    {
        for (const auto &id : mgr->documents()) {
            mgr->removeDocument(id);
        }
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

    void testExportFile()
    {
        PkPassManager passMgr;
        clearPasses(&passMgr);

        ReservationManager resMgr;
        clearReservations(&resMgr);

        DocumentManager docMgr;
        clearDocuments(&docMgr);

        TripGroupManager tripGroupMgr;
        tripGroupMgr.setReservationManager(&resMgr);
        TransferManager transferMgr;
        transferMgr.setReservationManager(&resMgr);
        transferMgr.setTripGroupManager(&tripGroupMgr);

        FavoriteLocationModel favLoc;

        HealthCertificateManager healthCertMgr;

        ApplicationController appController;
        appController.setPkPassManager(&passMgr);
        appController.setReservationManager(&resMgr);
        appController.setDocumentManager(&docMgr);
        appController.setTransferManager(&transferMgr);
        appController.setFavoriteLocationModel(&favLoc);
        appController.setHealthCertificateManager(&healthCertMgr);

        appController.importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/4U8465-v1.json")));
        appController.importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        QCOMPARE(resMgr.batches().size(), 1);
        QCOMPARE(passMgr.passes().size(), 1);

        QTemporaryFile tmp;
        QVERIFY(tmp.open());
        tmp.close();
        appController.exportToFile(tmp.fileName());

        KItinerary::File f(tmp.fileName());
        QVERIFY(f.open(KItinerary::File::Read));
        QCOMPARE(f.reservations().size(), 1);
        QCOMPARE(f.passes().size(), 1);

        clearPasses(&passMgr);
        clearReservations(&resMgr);
        clearDocuments(&docMgr);
        QCOMPARE(resMgr.batches().size(), 0);
        QCOMPARE(passMgr.passes().size(), 0);

        appController.importFromUrl(QUrl::fromLocalFile(tmp.fileName()));
        QCOMPARE(resMgr.batches().size(), 1);
        QCOMPARE(passMgr.passes().size(), 1);

        clearPasses(&passMgr);
        clearReservations(&resMgr);
        QCOMPARE(resMgr.batches().size(), 0);
        QCOMPARE(passMgr.passes().size(), 0);

        QFile bundle(tmp.fileName());
        QVERIFY(bundle.open(QFile::ReadOnly));
        appController.importData(bundle.readAll());
        QCOMPARE(resMgr.batches().size(), 1);
        QCOMPARE(passMgr.passes().size(), 1);
    }
};

QTEST_GUILESS_MAIN(AppControllerTest)

#include "applicationcontrollertest.moc"
