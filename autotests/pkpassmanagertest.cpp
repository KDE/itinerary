/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"
#include "mocknetworkaccessmanager.h"

#include "pkpassmanager.h"

#include <QUrl>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("LANG", "C");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

static MockNetworkAccessManager s_nam;
static QNetworkAccessManager* namFactory() { return &s_nam; }

class PkPassManagerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testImport()
    {
        PkPassManager mgr;
        Test::clearAll(&mgr);
        const auto now = QDateTime::currentDateTime().addSecs(-1);

        QSignalSpy addSpy(&mgr, &PkPassManager::passAdded);
        QVERIFY(addSpy.isValid());
        QSignalSpy updateSpy(&mgr, &PkPassManager::passUpdated);
        QVERIFY(updateSpy.isValid());
        QSignalSpy rmSpy(&mgr, &PkPassManager::passRemoved);
        QVERIFY(rmSpy.isValid());

        const auto passId = QStringLiteral("pass.booking.kde.org/MTIzNA==");
        QVERIFY(mgr.passes().isEmpty());
        QCOMPARE(mgr.pass(passId), nullptr);
        QVERIFY(!mgr.updateTime(passId).isValid());

        mgr.importPass(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(addSpy.at(0).at(0).toString(), passId);
        QVERIFY(updateSpy.isEmpty());
        QVERIFY(mgr.pass(passId));
        QVERIFY(mgr.updateTime(passId) >= now);

        auto passes = mgr.passes();
        QCOMPARE(passes.size(), 1);
        QCOMPARE(passes.at(0), passId);

        mgr.importPass(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/boardingpass-v2.pkpass")));
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(mgr.passes().size(), 1);
        QCOMPARE(updateSpy.at(0).at(0).toString(), passId);
        QCOMPARE(updateSpy.at(0).at(1).toStringList().size(), 1);
        QCOMPARE(updateSpy.at(0).at(1).toStringList().at(0), QStringLiteral("Gate changed to G30."));
        QVERIFY(mgr.pass(passId));

        Test::clearAll(&mgr);
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(rmSpy.size(), 1);
        QCOMPARE(rmSpy.at(0).at(0).toString(), passId);
        QVERIFY(mgr.passes().isEmpty());
        QCOMPARE(mgr.pass(passId), nullptr);
    }

    void testCaptivePortalFailure()
    {
        PkPassManager mgr;
        mgr.setNetworkAccessManagerFactory(namFactory);
        Test::clearAll(&mgr);
        QSignalSpy addSpy(&mgr, &PkPassManager::passAdded);
        QSignalSpy updateSpy(&mgr, &PkPassManager::passUpdated);

        mgr.importPass(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/updateable-boardingpass.pkpass")));
        QCOMPARE(mgr.passes().size(), 1);
        QCOMPARE(addSpy.size(), 1);
        addSpy.clear();
        const auto passId = mgr.passes()[0];
        QVERIFY(mgr.canUpdate(mgr.pass(passId)));

        s_nam.replies.push({QNetworkReply::NoError, 200, QByteArray("hello from the captive portal!"), QString()});

        mgr.updatePass(passId);
        QTest::qWait(0);

        QCOMPARE(addSpy.size(), 0);
        QCOMPARE(updateSpy.size(), 0);
    }
};

QTEST_GUILESS_MAIN(PkPassManagerTest)

#include "pkpassmanagertest.moc"
