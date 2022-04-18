/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"
#include <passmanager.h>

#include <KItinerary/JsonLdDocument>
#include <KItinerary/ProgramMembership>

#include <qtest.h>
#include <QAbstractItemModelTester>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryFile>

using namespace KItinerary;

class PassManagerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
        QStandardPaths::setTestModeEnabled(true);
    }

    void testPassManager()
    {
        PassManager mgr;
        QAbstractItemModelTester modelTest(&mgr);

        Test::clearAll(&mgr);
        QCOMPARE(mgr.rowCount(), 0);

        // test import
        QVERIFY(mgr.import(JsonLdDocument::fromJsonSingular(QJsonDocument::fromJson(Test::readFile(QStringLiteral(SOURCE_DIR "/data/bahncard.json"))).object())));
        QCOMPARE(mgr.rowCount(), 1);

        // retrieval
        auto idx = mgr.index(0, 0);
        const auto passId = idx.data(PassManager::PassIdRole).toString();
        QVERIFY(!passId.isEmpty());
        const auto pass = idx.data(PassManager::PassRole);
        QVERIFY(!pass.isNull());
        QVERIFY(JsonLd::isA<ProgramMembership>(pass));
        QCOMPARE(idx.data(PassManager::PassTypeRole).toInt(), PassManager::ProgramMembership);
        QVERIFY(!idx.data(PassManager::PassDataRole).toByteArray().isEmpty());

        {
            // test persistence
            PassManager mgr2;
            QCOMPARE(mgr2.rowCount(), 1);
            auto idx = mgr2.index(0, 0);
            QCOMPARE(idx.data(PassManager::PassIdRole).toString(), passId);
            const auto pass = idx.data(PassManager::PassRole);
            QVERIFY(!pass.isNull());
            QVERIFY(JsonLd::isA<ProgramMembership>(pass));
        }

        // test removal
        QVERIFY(mgr.remove(passId));
        QCOMPARE(mgr.rowCount(), 0);
    }
};

QTEST_GUILESS_MAIN(PassManagerTest)

#include "passmanagertest.moc"
