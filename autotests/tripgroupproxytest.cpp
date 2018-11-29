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

#include "modelverificationpoint.h"

#include <reservationmanager.h>
#include <timelinemodel.h>
#include <tripgroupmanager.h>
#include <tripgroupproxymodel.h>

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
#include <QAbstractItemModelTester>
#endif
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

class TripGroupProxyTest : public QObject
{
    Q_OBJECT
private:
    void clearReservations(ReservationManager *mgr)
    {
        for (const auto &id : mgr->reservations()) {
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
        qputenv("LC_ALL", "en_US.utf-8");
        QStandardPaths::setTestModeEnabled(true);
    }

    void init()
    {
        TripGroupManager::clear();
    }

    void testExpandCollapse()
    {
        ReservationManager resMgr;
        clearReservations(&resMgr);
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/timeline/multi-traveler-merge-with-countryinfo.json")));
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        resMgr.importReservation(readFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));

        TripGroupManager groupMgr;
        QSignalSpy addSpy(&groupMgr, &TripGroupManager::tripGroupAdded);
        groupMgr.setReservationManager(&resMgr);
        QCOMPARE(groupMgr.tripGroups().size(), 3);
        QCOMPARE(addSpy.size(), 3);

        TimelineModel model;
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setCurrentDateTime(QDateTime({1996, 10, 14}, {12, 34}));
        model.setReservationManager(&resMgr);
        model.setTripGroupManager(&groupMgr);

        TripGroupProxyModel proxy;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester tester(&proxy);
#endif
        proxy.setSourceModel(&model);
        proxy.expand(addSpy.at(0).at(0).toString());
        proxy.expand(addSpy.at(1).at(0).toString());
        proxy.expand(addSpy.at(2).at(0).toString());

        ModelVerificationPoint vp0(QLatin1String(SOURCE_DIR "/data/tripgroupproxy/expand-collapse-r0.model"));
        vp0.setRoleFilter({TimelineModel::ReservationIdsRole, TimelineModel::TripGroupIdRole});
        QVERIFY(vp0.verify(&proxy));

        proxy.collapse(addSpy.at(0).at(0).toString());
        ModelVerificationPoint vp1(QLatin1String(SOURCE_DIR "/data/tripgroupproxy/expand-collapse-r1.model"));
        vp1.setRoleFilter({TimelineModel::ReservationIdsRole, TimelineModel::TripGroupIdRole});
        QVERIFY(vp1.verify(&proxy));

        proxy.collapse(addSpy.at(1).at(0).toString());
        ModelVerificationPoint vp2(QLatin1String(SOURCE_DIR "/data/tripgroupproxy/expand-collapse-r2.model"));
        vp2.setRoleFilter({TimelineModel::ReservationIdsRole, TimelineModel::TripGroupIdRole});
        QVERIFY(vp2.verify(&proxy));

        proxy.collapse(addSpy.at(2).at(0).toString());
        ModelVerificationPoint vp3(QLatin1String(SOURCE_DIR "/data/tripgroupproxy/expand-collapse-r3.model"));
        vp3.setRoleFilter({TimelineModel::ReservationIdsRole, TimelineModel::TripGroupIdRole});
        QVERIFY(vp3.verify(&proxy));

        proxy.expand(addSpy.at(2).at(0).toString());
        QVERIFY(vp2.verify(&proxy));
        proxy.expand(addSpy.at(1).at(0).toString());
        QVERIFY(vp1.verify(&proxy));
        proxy.expand(addSpy.at(0).at(0).toString());
        QVERIFY(vp0.verify(&proxy));
    }
};
QTEST_GUILESS_MAIN(TripGroupProxyTest)

#include "tripgroupproxytest.moc"
