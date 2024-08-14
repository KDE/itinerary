/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "modelverificationpoint.h"
#include "testhelper.h"

#include "applicationcontroller.h"
#include "importcontroller.h"
#include "reservationmanager.h"
#include "transfermanager.h"
#include "timelinemodel.h"
#include "tripgroupmanager.h"
#include "tripgroupproxymodel.h"

#include <QAbstractItemModelTester>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

using namespace Qt::Literals;

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
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/timeline/multi-traveler-merge-with-countryinfo.json")));
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/google-multi-passenger-flight.json")));
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);

        TripGroupManager groupMgr;
        QSignalSpy addSpy(&groupMgr, &TripGroupManager::tripGroupAdded);
        groupMgr.setReservationManager(&resMgr);
        groupMgr.setTransferManager(&transferMgr);
        QCOMPARE(groupMgr.tripGroups().size(), 3);
        QCOMPARE(addSpy.size(), 3);

        TimelineModel model;
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setCurrentDateTime(QDateTime({1996, 10, 14}, {12, 34}));
        model.setReservationManager(&resMgr);
        model.setTransferManager(&transferMgr);
        model.setTripGroupManager(&groupMgr);
        Test::waitForReset(&model);

        TripGroupProxyModel proxy;
        QAbstractItemModelTester tester(&proxy);
        proxy.setSourceModel(&model);
        proxy.expand(addSpy.at(0).at(0).toString());
        proxy.expand(addSpy.at(1).at(0).toString());
        proxy.expand(addSpy.at(2).at(0).toString());

        ModelVerificationPoint vp0(QLatin1StringView(SOURCE_DIR "/data/tripgroupproxy/expand-collapse-r0.model"));
        vp0.setRoleFilter({TimelineModel::BatchIdRole, TimelineModel::TripGroupIdRole});
        vp0.setJsonPropertyFilter({"elements"_L1});
        QVERIFY(vp0.verify(&proxy));

        proxy.collapse(addSpy.at(0).at(0).toString());
        ModelVerificationPoint vp1(QLatin1StringView(SOURCE_DIR "/data/tripgroupproxy/expand-collapse-r1.model"));
        vp1.setRoleFilter({TimelineModel::BatchIdRole, TimelineModel::TripGroupIdRole});
        vp1.setJsonPropertyFilter({"elements"_L1});
        QVERIFY(vp1.verify(&proxy));

        proxy.collapse(addSpy.at(1).at(0).toString());
        ModelVerificationPoint vp2(QLatin1StringView(SOURCE_DIR "/data/tripgroupproxy/expand-collapse-r2.model"));
        vp2.setRoleFilter({TimelineModel::BatchIdRole, TimelineModel::TripGroupIdRole});
        vp2.setJsonPropertyFilter({"elements"_L1});
        QVERIFY(vp2.verify(&proxy));

        proxy.collapse(addSpy.at(2).at(0).toString());
        ModelVerificationPoint vp3(QLatin1StringView(SOURCE_DIR "/data/tripgroupproxy/expand-collapse-r3.model"));
        vp3.setRoleFilter({TimelineModel::BatchIdRole, TimelineModel::TripGroupIdRole});
        vp3.setJsonPropertyFilter({"elements"_L1});
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
        TransferManager transferMgr;
        Test::clearAll(&resMgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&resMgr);
        ImportController importer;
        importer.setReservationManager(&resMgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);

        TripGroupManager groupMgr;
        QSignalSpy addSpy(&groupMgr, &TripGroupManager::tripGroupAdded);
        groupMgr.setReservationManager(&resMgr);
        groupMgr.setTransferManager(&transferMgr);
        QCOMPARE(groupMgr.tripGroups().size(), 1);
        QCOMPARE(addSpy.size(), 1);

        TimelineModel model;
        model.setHomeCountryIsoCode(QStringLiteral("DE"));
        model.setCurrentDateTime(QDateTime({2017, 9, 9}, {12, 34}));
        model.setReservationManager(&resMgr);
        model.setTransferManager(&transferMgr);
        model.setTripGroupManager(&groupMgr);

        TripGroupProxyModel proxy;
        QAbstractItemModelTester tester(&proxy);
        proxy.setSourceModel(&model);
        Test::waitForReset(&model);

        // future event, should be expanded
        ModelVerificationPoint vp0(QLatin1StringView(SOURCE_DIR "/data/tripgroupproxy/current-r0.model"));
        vp0.setRoleFilter({TimelineModel::BatchIdRole, TimelineModel::TripGroupIdRole});
        vp0.setJsonPropertyFilter({"elements"_L1});
        QVERIFY(vp0.verify(&proxy));

        // current event, must be expanded and not collapsible
        model.setCurrentDateTime(QDateTime({2017, 9, 14}, {12, 34}));
        ModelVerificationPoint vp1(QLatin1StringView(SOURCE_DIR "/data/tripgroupproxy/current-r1.model"));
        vp1.setRoleFilter({TimelineModel::BatchIdRole, TimelineModel::TripGroupIdRole});
        vp1.setJsonPropertyFilter({"elements"_L1});
        QVERIFY(vp1.verify(&proxy));
        proxy.collapse(addSpy.at(0).at(0).toString());
        QVERIFY(vp1.verify(&proxy));

        // past event, should be collapsed
        model.setCurrentDateTime(QDateTime({2018, 9, 9}, {12, 34}));
        ModelVerificationPoint vp2(QLatin1StringView(SOURCE_DIR "/data/tripgroupproxy/current-r2.model"));
        vp2.setRoleFilter({TimelineModel::BatchIdRole, TimelineModel::TripGroupIdRole});
        vp2.setJsonPropertyFilter({"elements"_L1});
        QCOMPARE(proxy.rowCount(), 3);
        QVERIFY(vp2.verify(&proxy));
    }

};
QTEST_GUILESS_MAIN(TripGroupProxyTest)

#include "tripgroupproxytest.moc"
