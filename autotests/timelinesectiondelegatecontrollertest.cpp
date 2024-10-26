/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "applicationcontroller.h"
#include "importcontroller.h"
#include "reservationmanager.h"
#include "timelinesectiondelegatecontroller.h"
#include "transfermanager.h"
#include "tripgroupmanager.h"
#include "tripgroupmodel.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QTimeZone>
#include <QUrl>
#include <QtTest/qtest.h>

using namespace KItinerary;

void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
    qputenv("TZ", "UTC");
    qputenv("LANG", "en_US");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class TimelineSectionDelegateControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testController()
    {
        ReservationManager mgr;
        Test::clearAll(&mgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&mgr);

        TransferManager transferMgr;

        TripGroupManager tgMgr;
        tgMgr.setReservationManager(&mgr);
        tgMgr.setTransferManager(&transferMgr);
        ctrl->setTripGroupManager(&tgMgr);

        ImportController importer;
        importer.setReservationManager(&mgr);
        importer.importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));
        ctrl->commitImport(&importer);

        TripGroupModel model;
        model.setTripGroupManager(&tgMgr);
        model.setCurrentDateTime(QDateTime({2021, 3, 7}, {8, 0}));
        QCOMPARE(model.rowCount(), 1);

        TimelineSectionDelegateController controller;
        controller.setTripGroupModel(&model);

        controller.setDateString(QStringLiteral("2021-03-07"));
        QCOMPARE(controller.isToday(), true);
        QCOMPARE(controller.title(), QLatin1StringView("Today"));
        QCOMPARE(controller.subTitle(), QString());
        QCOMPARE(controller.isHoliday(), false);

        controller.setDateString(QStringLiteral("2021-03-08"));
        QCOMPARE(controller.isToday(), false);
        QCOMPARE(controller.title(), QLatin1StringView("Monday, 3/8/21"));
        QCOMPARE(controller.subTitle(), QLatin1StringView("Internationaler Frauentag")); // DE-BE only, so proves the right region was selected
        QCOMPARE(controller.isHoliday(), true);

        model.setCurrentDateTime(QDateTime({2021, 12, 27}, {8, 0}));
        controller.setDateString(QStringLiteral("2021-12-31")); // named date, but not a bank holiday
        QCOMPARE(controller.isToday(), false);
        QCOMPARE(controller.subTitle(), QLatin1StringView("Silvester"));
        QCOMPARE(controller.isHoliday(), false);
        controller.setDateString(QStringLiteral("2022-01-01"));
        QCOMPARE(controller.isToday(), false);
        QCOMPARE(controller.subTitle(), QLatin1StringView("Neujahr"));
        QCOMPARE(controller.isHoliday(), true);
    }
};

QTEST_GUILESS_MAIN(TimelineSectionDelegateControllerTest)

#include "timelinesectiondelegatecontrollertest.moc"
