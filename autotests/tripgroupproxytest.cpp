/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "modelverificationpoint.h"

#include <applicationcontroller.h>
#include <reservationmanager.h>
#include <timelinemodel.h>
#include <tripgroupmanager.h>
#include <tripgroupproxymodel.h>

#include <QAbstractItemModelTester>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("LANG", "en_US");
    qputenv("TZ", "UTC");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class TripGroupProxyTest : public QObject
{
    Q_OBJECT
private:
    void clearReservations(ReservationManager *mgr)
    {
        const auto batches = mgr->batches(); // copy, as this is getting modified in the process
        for (const auto &id : batches) {
            mgr->removeBatch(id);
        }
        QCOMPARE(mgr->batches().size(), 0);
    }

private Q_SLOTS:
    void initTestCase()
    {
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
        ApplicationController ctrl;
        ctrl.setReservationManager(&resMgr);
        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/timeline/multi-traveler-merge-with-countryinfo.json")));
        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));

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
        QAbstractItemModelTester tester(&proxy);
        proxy.setSourceModel(&model);
        proxy.expand(addSpy.at(0).at(0).toString());
        proxy.expand(addSpy.at(1).at(0).toString());
        proxy.expand(addSpy.at(2).at(0).toString());

        ModelVerificationPoint vp0(QLatin1String(SOURCE_DIR "/data/tripgroupproxy/expand-collapse-r0.model"));
        vp0.setRoleFilter({TimelineModel::BatchIdRole, TimelineModel::TripGroupIdRole});
        QVERIFY(vp0.verify(&proxy));

        proxy.collapse(addSpy.at(0).at(0).toString());
        ModelVerificationPoint vp1(QLatin1String(SOURCE_DIR "/data/tripgroupproxy/expand-collapse-r1.model"));
        vp1.setRoleFilter({TimelineModel::BatchIdRole, TimelineModel::TripGroupIdRole});
        QVERIFY(vp1.verify(&proxy));

        proxy.collapse(addSpy.at(1).at(0).toString());
        ModelVerificationPoint vp2(QLatin1String(SOURCE_DIR "/data/tripgroupproxy/expand-collapse-r2.model"));
        vp2.setRoleFilter({TimelineModel::BatchIdRole, TimelineModel::TripGroupIdRole});
        QVERIFY(vp2.verify(&proxy));

        proxy.collapse(addSpy.at(2).at(0).toString());
        ModelVerificationPoint vp3(QLatin1String(SOURCE_DIR "/data/tripgroupproxy/expand-collapse-r3.model"));
        vp3.setRoleFilter({TimelineModel::BatchIdRole, TimelineModel::TripGroupIdRole});
        QVERIFY(vp3.verify(&proxy));

        proxy.expand(addSpy.at(2).at(0).toString());
        QVERIFY(vp2.verify(&proxy));
        proxy.expand(addSpy.at(1).at(0).toString());
        QVERIFY(vp1.verify(&proxy));
        proxy.expand(addSpy.at(0).at(0).toString());
        QVERIFY(vp0.verify(&proxy));
    }

    void testCurrentGroup()
    {
        ReservationManager resMgr;
        clearReservations(&resMgr);
        ApplicationController ctrl;
        ctrl.setReservationManager(&resMgr);
        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));

        TripGroupManager groupMgr;
        QSignalSpy addSpy(&groupMgr, &TripGroupManager::tripGroupAdded);
        groupMgr.setReservationManager(&resMgr);
        QCOMPARE(groupMgr.tripGroups().size(), 1);
        QCOMPARE(addSpy.size(), 1);

        TimelineModel model;
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setCurrentDateTime(QDateTime({2017, 9, 9}, {12, 34}));
        model.setReservationManager(&resMgr);
        model.setTripGroupManager(&groupMgr);

        TripGroupProxyModel proxy;
        QAbstractItemModelTester tester(&proxy);
        proxy.setSourceModel(&model);

        // future event, should be expanded
        ModelVerificationPoint vp0(QLatin1String(SOURCE_DIR "/data/tripgroupproxy/current-r0.model"));
        vp0.setRoleFilter({TimelineModel::BatchIdRole, TimelineModel::TripGroupIdRole});
        QVERIFY(vp0.verify(&proxy));

        // current event, must be expanded and not collapsible
        model.setCurrentDateTime(QDateTime({2017, 9, 14}, {12, 34}));
        ModelVerificationPoint vp1(QLatin1String(SOURCE_DIR "/data/tripgroupproxy/current-r1.model"));
        vp1.setRoleFilter({TimelineModel::BatchIdRole, TimelineModel::TripGroupIdRole});
        QVERIFY(vp1.verify(&proxy));
        proxy.collapse(addSpy.at(0).at(0).toString());
        QVERIFY(vp1.verify(&proxy));

        // past event, should be collapsed
        model.setCurrentDateTime(QDateTime({2018, 9, 9}, {12, 34}));
        ModelVerificationPoint vp2(QLatin1String(SOURCE_DIR "/data/tripgroupproxy/current-r2.model"));
        vp2.setRoleFilter({TimelineModel::BatchIdRole, TimelineModel::TripGroupIdRole});
        QCOMPARE(proxy.rowCount(), 2);
        QVERIFY(vp2.verify(&proxy));
    }

};
QTEST_GUILESS_MAIN(TripGroupProxyTest)

#include "tripgroupproxytest.moc"
