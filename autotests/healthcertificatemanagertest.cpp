/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "healthcertificatemanager.h"
#include "applicationcontroller.h"
#include "importcontroller.h"
#include "pkpassmanager.h"
#include "reservationmanager.h"
#include <config-itinerary.h>

#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QtTest/qtest.h>

class HealthCertificateManagerTest : public QObject
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
        QStandardPaths::setTestModeEnabled(true);
    }

    void testManager()
    {
        PkPassManager passMgr;
        ReservationManager resMgr;

        {
            HealthCertificateManager mgr;
            QAbstractItemModelTester modelTester(&mgr);
            // clear previous content, if any
            while (mgr.rowCount()) {
                mgr.removeCertificate(0);
            }
        }

        {
            ApplicationController ctrl;
            QSignalSpy infoSpy(&ctrl, &ApplicationController::infoMessage);
            ctrl.setPkPassManager(&passMgr);
            ctrl.setReservationManager(&resMgr);
            auto mgr = ctrl.healthCertificateManager();
            QAbstractItemModelTester modelTester(mgr);
            QSignalSpy insertSpy(mgr, &QAbstractItemModel::rowsInserted);
            QCOMPARE(mgr->rowCount(), 0);
            const auto rawData = readFile(QLatin1StringView(SOURCE_DIR "/data/health-certificates/full-vaccination.txt"));

            ImportController importer;
            importer.importData(rawData);
#if HAVE_KHEALTHCERTIFICATE
            ctrl.commitImport(&importer);
            QCOMPARE(mgr->rowCount(), 1);
            QCOMPARE(insertSpy.size(), 1);
            QVERIFY(!mgr->data(mgr->index(0, 0), Qt::DisplayRole).toString().isEmpty());
            QVERIFY(!mgr->data(mgr->index(0, 0), HealthCertificateManager::CertificateRole).isNull());
            QCOMPARE(mgr->data(mgr->index(0, 0), HealthCertificateManager::RawDataRole).toByteArray(), rawData);
            QVERIFY(!mgr->data(mgr->index(0, 0), HealthCertificateManager::StorageIdRole).toString().isEmpty());

            QCOMPARE(infoSpy.size(), 1);
#endif
        }

        {
            ApplicationController ctrl;
            QSignalSpy infoSpy(&ctrl, &ApplicationController::infoMessage);
            ctrl.setPkPassManager(&passMgr);
            ctrl.setReservationManager(&resMgr);
            auto mgr = ctrl.healthCertificateManager();
            QAbstractItemModelTester modelTester(mgr);
#if HAVE_KHEALTHCERTIFICATE
            QCOMPARE(mgr->rowCount(), 1);
            const auto rawData = readFile(QLatin1StringView(SOURCE_DIR "/data/health-certificates/full-vaccination.txt"));
            // no duplicates
            ImportController importer;
            importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/health-certificates/full-vaccination.txt")));
            ctrl.commitImport(&importer);
            QCOMPARE(mgr->rowCount(), 1);

            QCOMPARE(infoSpy.size(), 1);
#endif
        }

        {
            ApplicationController ctrl;
            ImportController importer;
            QSignalSpy infoSpy(&importer, &ImportController::infoMessage);
            ctrl.setPkPassManager(&passMgr);
            ctrl.setReservationManager(&resMgr);
            auto mgr = ctrl.healthCertificateManager();
            QAbstractItemModelTester modelTester(mgr);
#if HAVE_KHEALTHCERTIFICATE
            QCOMPARE(mgr->rowCount(), 1);
            importer.importData("not a vaccination certificate");
            // garbage is rejected
            ctrl.commitImport(&importer);
            QCOMPARE(mgr->rowCount(), 1);

            QCOMPARE(infoSpy.size(), 1);
#endif
        }

        {
            ApplicationController ctrl;
            QSignalSpy infoSpy(&ctrl, &ApplicationController::infoMessage);
            ctrl.setPkPassManager(&passMgr);
            ctrl.setReservationManager(&resMgr);
            auto mgr = ctrl.healthCertificateManager();
            QAbstractItemModelTester modelTester(mgr);
            QSignalSpy insertSpy(mgr, &QAbstractItemModel::rowsInserted);

#if HAVE_KHEALTHCERTIFICATE
            QCOMPARE(mgr->rowCount(), 1);
            const auto rawData = readFile(QLatin1StringView(SOURCE_DIR "/data/health-certificates/partial-vaccination.divoc"));
            ImportController importer;
            importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/health-certificates/partial-vaccination.divoc")));
            ctrl.commitImport(&importer);
            QCOMPARE(mgr->rowCount(), 2);
            QCOMPARE(insertSpy.size(), 1);
            QVERIFY(!mgr->data(mgr->index(0, 0), Qt::DisplayRole).toString().isEmpty());
            QVERIFY(!mgr->data(mgr->index(0, 0), HealthCertificateManager::CertificateRole).isNull());
            QCOMPARE(mgr->data(mgr->index(0, 0), HealthCertificateManager::RawDataRole).toByteArray(), rawData);
            QVERIFY(!mgr->data(mgr->index(0, 0), HealthCertificateManager::StorageIdRole).toString().isEmpty());

            QCOMPARE(infoSpy.size(), 1);
#endif
        }

        {
            ApplicationController ctrl;
            QSignalSpy infoSpy(&ctrl, &ApplicationController::infoMessage);
            ctrl.setPkPassManager(&passMgr);
            ctrl.setReservationManager(&resMgr);
            auto mgr = ctrl.healthCertificateManager();
            QAbstractItemModelTester modelTester(mgr);
            QSignalSpy insertSpy(mgr, &QAbstractItemModel::rowsInserted);
#if HAVE_KHEALTHCERTIFICATE
            QCOMPARE(mgr->rowCount(), 2);
            ImportController importer;
            importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/health-certificates/negative-pcr-test-fr.pdf")));
            ctrl.commitImport(&importer);
            QCOMPARE(mgr->rowCount(), 3);
            QCOMPARE(insertSpy.size(), 1);

            QCOMPARE(infoSpy.size(), 1);
#endif
        }

        {
            HealthCertificateManager mgr;
            QAbstractItemModelTester modelTester(&mgr);
            QSignalSpy removeSpy(&mgr, &QAbstractItemModel::rowsRemoved);
#if HAVE_KHEALTHCERTIFICATE
            QCOMPARE(mgr.rowCount(), 3);
            mgr.removeCertificate(0);
            QCOMPARE(removeSpy.size(), 1);
            QCOMPARE(mgr.rowCount(), 2);
#endif
        }
    }
};

QTEST_GUILESS_MAIN(HealthCertificateManagerTest)

#include "healthcertificatemanagertest.moc"
