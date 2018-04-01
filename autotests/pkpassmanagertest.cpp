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

#include <pkpassmanager.h>

#include <QUrl>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

class PkPassManagerTest : public QObject
{
    Q_OBJECT
private:
    void clearPasses(PkPassManager *mgr)
    {
        for (const auto id : mgr->passes())
            mgr->removePass(id);
    }

private slots:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testImport()
    {
        PkPassManager mgr;
        clearPasses(&mgr);

        QSignalSpy addSpy(&mgr, &PkPassManager::passAdded);
        QVERIFY(addSpy.isValid());
        QSignalSpy updateSpy(&mgr, &PkPassManager::passUpdated);
        QVERIFY(updateSpy.isValid());
        QSignalSpy rmSpy(&mgr, &PkPassManager::passRemoved);
        QVERIFY(rmSpy.isValid());

        const auto passId = QStringLiteral("pass.booking.kde.org/MTIzNA==");
        QVERIFY(mgr.passes().isEmpty());
        QCOMPARE(mgr.pass(passId), nullptr);

        mgr.importPass(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(addSpy.at(0).at(0).toString(), passId);
        QVERIFY(updateSpy.isEmpty());
        QVERIFY(mgr.pass(passId));

        auto passes = mgr.passes();
        QCOMPARE(passes.size(), 1);
        QCOMPARE(passes.at(0), passId);

        mgr.importPass(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v2.pkpass")));
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(mgr.passes().size(), 1);
        QCOMPARE(updateSpy.at(0).at(0).toString(), passId);
        QCOMPARE(updateSpy.at(0).at(1).toStringList().size(), 1);
        QCOMPARE(updateSpy.at(0).at(1).toStringList().at(0), QStringLiteral("Gate changed to G30."));
        QVERIFY(mgr.pass(passId));

        clearPasses(&mgr);
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(rmSpy.size(), 1);
        QCOMPARE(rmSpy.at(0).at(0).toString(), passId);
        QVERIFY(mgr.passes().isEmpty());
        QCOMPARE(mgr.pass(passId), nullptr);
    }
};

QTEST_GUILESS_MAIN(PkPassManagerTest)

#include "pkpassmanagertest.moc"
