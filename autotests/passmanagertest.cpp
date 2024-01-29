/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"
#include "passmanager.h"

#include <KItinerary/JsonLdDocument>
#include <KItinerary/ProgramMembership>
#include <KItinerary/Ticket>

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
        QVERIFY(!mgr.import(JsonLdDocument::fromJsonSingular(QJsonDocument::fromJson(Test::readFile(QStringLiteral(SOURCE_DIR "/data/bahncard.json"))).object())).isEmpty());
        QVERIFY(!mgr.import(JsonLdDocument::fromJsonSingular(QJsonDocument::fromJson(Test::readFile(QStringLiteral(SOURCE_DIR "/data/9euroticket.json"))).object())).isEmpty());
        // duplicates get merged
        QVERIFY(!mgr.import(JsonLdDocument::fromJsonSingular(QJsonDocument::fromJson(Test::readFile(QStringLiteral(SOURCE_DIR "/data/9euroticket.json"))).object())).isEmpty());
        QCOMPARE(mgr.rowCount(), 2);

        // retrieval
        auto idx = mgr.index(0, 0);
        const auto passId = idx.data(PassManager::PassIdRole).toString();
        QVERIFY(!passId.isEmpty());
        const auto pass = idx.data(PassManager::PassRole);
        QVERIFY(!pass.isNull());
        QVERIFY(JsonLd::isA<ProgramMembership>(pass));
        QCOMPARE(idx.data(PassManager::PassTypeRole).toInt(), PassManager::ProgramMembership);
        QVERIFY(!idx.data(PassManager::PassDataRole).toByteArray().isEmpty());
        QCOMPARE(idx.data(PassManager::NameRole).toString(), QLatin1String("BahnCard 25 (2. Kl.) (BC25)"));
        QCOMPARE(idx.data(PassManager::ValidUntilRole).toDateTime(), QDateTime({2122, 3, 24}, {23, 59, 59}));
        QVERIFY(!idx.data(PassManager::SectionRole).toString().isEmpty());
        QVERIFY(!mgr.pass(passId).isNull());

        idx = mgr.index(1, 0);
        const auto passId2 = idx.data(PassManager::PassIdRole).toString();
        QVERIFY(!passId2.isEmpty());
        const auto ticket = idx.data(PassManager::PassRole);
        QVERIFY(!ticket.isNull());
        QVERIFY(JsonLd::isA<Ticket>(ticket));
        QCOMPARE(idx.data(PassManager::PassTypeRole).toInt(), PassManager::Ticket);
        QVERIFY(!idx.data(PassManager::PassDataRole).toByteArray().isEmpty());
        QCOMPARE(idx.data(PassManager::NameRole).toString(), QLatin1String("9-Euro-Ticket"));
        QCOMPARE(idx.data(PassManager::ValidUntilRole).toDateTime(), QDateTime({2022, 5, 31}, {23, 59, 59}));
        QVERIFY(!idx.data(PassManager::SectionRole).toString().isEmpty());
        QVERIFY(!mgr.pass(passId2).isNull());

        mgr.remove(passId2);
        QCOMPARE(mgr.rowCount(), 1);



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

    void testPassMatching_data()
    {
        QTest::addColumn<QString>("name");
        QTest::addColumn<QString>("number");
        QTest::addColumn<bool>("matches");

        QTest::newRow("empty") << QString() << QString() << false;
        QTest::newRow("generic-name") << QStringLiteral("BahnCard") << QString() << true;
        QTest::newRow("generic-name-case-mismatch") << QStringLiteral("Bahncard") << QString() << true;
        QTest::newRow("number-match") << QStringLiteral("BahnCard") << QStringLiteral("7081123456789012") << true;
        QTest::newRow("number-mismatch") << QStringLiteral("BahnCard") << QStringLiteral("7081123456789013") << false;
        QTest::newRow("name-mismatch") << QStringLiteral("CartaFRECCIA") << QString() << false;
        QTest::newRow("abbr-match") << QStringLiteral("BC25") << QString() << true;
        QTest::newRow("abbr-mismatch") << QStringLiteral("BC50") << QString() << false;
    }

    void testPassMatching()
    {
        QFETCH(QString, name);
        QFETCH(QString, number);
        QFETCH(bool, matches);

        PassManager mgr;
        Test::clearAll(&mgr);
        QVERIFY(!mgr.import(JsonLdDocument::fromJsonSingular(QJsonDocument::fromJson(Test::readFile(QStringLiteral(SOURCE_DIR "/data/bahncard.json"))).object())).isEmpty());
        const auto passId = mgr.index(0, 0).data(PassManager::PassIdRole).toString();
        QVERIFY(!passId.isEmpty());

        ProgramMembership program;
        program.setProgramName(name);
        program.setMembershipNumber(number);
        if (matches) {
            QCOMPARE(mgr.findMatchingPass(program), passId);
        } else {
            QCOMPARE(mgr.findMatchingPass(program), QString());
        }
    }

    void testPassUpdate()
    {
        PassManager mgr;
        QAbstractItemModelTester modelTest(&mgr);
        QSignalSpy updateSpy(&mgr, &PassManager::passChanged);

        Test::clearAll(&mgr);
        QCOMPARE(mgr.rowCount(), 0);

        const auto passId = mgr.import(JsonLdDocument::fromJsonSingular(QJsonDocument::fromJson(Test::readFile(QStringLiteral(SOURCE_DIR "/data/9euroticket.json"))).object()));
        QVERIFY(!passId.isEmpty());

        auto pass = mgr.pass(passId).value<Ticket>();
        QCOMPARE(pass.name(), QLatin1String("9-Euro-Ticket"));
        pass.setName(QLatin1String("9-Euro-Ticket (May 2022)"));
        mgr.update(passId, pass);

        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(updateSpy[0][0].toString(), passId);

        pass = mgr.pass(passId).value<Ticket>();
        QCOMPARE(pass.name(), QLatin1String("9-Euro-Ticket (May 2022)"));
    }
};

QTEST_GUILESS_MAIN(PassManagerTest)

#include "passmanagertest.moc"
