/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-itinerary.h>
#include <healthcertificatemanager.h>

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
        {
            HealthCertificateManager mgr;
            QAbstractItemModelTester modelTester(&mgr);
            // clear previous content, if any
            while (mgr.rowCount()) {
                mgr.removeCertificate(0);
            }
        }

        {
            HealthCertificateManager mgr;
            QAbstractItemModelTester modelTester(&mgr);
            QSignalSpy insertSpy(&mgr, &QAbstractItemModel::rowsInserted);
            QCOMPARE(mgr.rowCount(), 0);
            const auto rawData = readFile(QLatin1String(SOURCE_DIR "/data/health-certificates/full-vaccination.txt"));
            mgr.importCertificate(rawData);
#if HAVE_KHEALTHCERTIFICATE
            QCOMPARE(mgr.rowCount(), 1);
            QCOMPARE(insertSpy.size(), 1);
            QVERIFY(!mgr.data(mgr.index(0, 0), Qt::DisplayRole).toString().isEmpty());
            QVERIFY(!mgr.data(mgr.index(0, 0), HealthCertificateManager::CertificateRole).isNull());
            QCOMPARE(mgr.data(mgr.index(0, 0), HealthCertificateManager::RawDataRole).toByteArray(),  rawData);
            QVERIFY(!mgr.data(mgr.index(0, 0), HealthCertificateManager::StorageIdRole).toString().isEmpty());
#endif
        }

        {
            HealthCertificateManager mgr;
            QAbstractItemModelTester modelTester(&mgr);
            QSignalSpy removeSpy(&mgr, &QAbstractItemModel::rowsRemoved);
#if HAVE_KHEALTHCERTIFICATE
            QCOMPARE(mgr.rowCount(), 1);
            mgr.removeCertificate(0);
            QCOMPARE(removeSpy.size(), 1);
            QCOMPARE(mgr.rowCount(), 0);
#endif
        }
    }
};

QTEST_GUILESS_MAIN(HealthCertificateManagerTest)

#include "healthcertificatemanagertest.moc"
