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
        QVERIFY(mgr.passes().isEmpty());

        QSignalSpy addSpy(&mgr, &PkPassManager::passAdded);
        QVERIFY(addSpy.isValid());
        QSignalSpy updateSpy(&mgr, &PkPassManager::passUpdated);
        QVERIFY(updateSpy.isValid());
        QSignalSpy rmSpy(&mgr, &PkPassManager::passRemoved);
        QVERIFY(rmSpy.isValid());

        QVERIFY(mgr.passes().isEmpty());
        mgr.importPass(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v1.pkpass")));
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(addSpy.at(0).at(0).toString(), QLatin1String("pass.booking.kde.org/MTIzNA=="));
        QVERIFY(updateSpy.isEmpty());

        auto passes = mgr.passes();
        QCOMPARE(passes.size(), 1);
        QCOMPARE(passes.at(0), QLatin1String("pass.booking.kde.org/MTIzNA=="));

        mgr.importPass(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/boardingpass-v2.pkpass")));
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(mgr.passes().size(), 1);
        QCOMPARE(updateSpy.at(0).at(0).toString(), QLatin1String("pass.booking.kde.org/MTIzNA=="));
        QCOMPARE(updateSpy.at(0).at(1).toStringList().size(), 1);
        QCOMPARE(updateSpy.at(0).at(1).toStringList().at(0), QStringLiteral("Gate changed to G30."));

        clearPasses(&mgr);
        QCOMPARE(addSpy.size(), 1);
        QCOMPARE(updateSpy.size(), 1);
        QCOMPARE(rmSpy.size(), 1);
        QCOMPARE(rmSpy.at(0).at(0).toString(), QLatin1String("pass.booking.kde.org/MTIzNA=="));
        QVERIFY(mgr.passes().isEmpty());
    }
};

QTEST_GUILESS_MAIN(PkPassManagerTest)

#include "pkpassmanagertest.moc"
