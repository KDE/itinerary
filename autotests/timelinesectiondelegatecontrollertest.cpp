/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "locationhelper.h"
#include "timelinemodel.h"
#include "timelinesectiondelegatecontroller.h"
#include "applicationcontroller.h"
#include "reservationmanager.h"

#include <QUrl>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTimeZone>

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
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/../tests/randa2017.json")));

        TimelineModel model;
        model.setCurrentDateTime(QDateTime({2021, 3, 7}, {8, 0}));
        model.setReservationManager(&mgr);

        QCOMPARE(model.locationAtTime(QDateTime({2017, 9, 10}, {0, 0})), QVariant());
        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2017, 9, 11}, {0, 0}))), QLatin1StringView("CH-VS"));
        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2017, 9, 16}, {0, 0}))), QLatin1StringView("DE-BE"));

        TimelineSectionDelegateController controller;
        controller.setTimelineModel(&model);

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

    void testBug455083()
    {
        ReservationManager mgr;
        Test::clearAll(&mgr);
        auto ctrl = Test::makeAppController();
        ctrl->setReservationManager(&mgr);
        // test data puts our known location to DE-BY and then adds a hotel in DE-BE for the BY-only public holiday on 2022-06-16
        ctrl->importFromUrl(QUrl::fromLocalFile(QLatin1StringView(SOURCE_DIR "/data/bug455083.json")));

        TimelineModel model;
        model.setCurrentDateTime(QDateTime({2022, 6, 14}, {8, 0}));
        model.setReservationManager(&mgr);

        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2022, 6, 12}, {0, 0}, QTimeZone("Europe/Berlin")))), QLatin1StringView("DE-BY"));
        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2022, 6, 13}, {0, 0}, QTimeZone("Europe/Berlin")))), QLatin1StringView("DE-BY"));
        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2022, 6, 14}, {0, 0}, QTimeZone("Europe/Berlin")))), QLatin1StringView("DE-BE"));
        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2022, 6, 16}, {0, 0}, QTimeZone("Europe/Berlin")))), QLatin1StringView("DE-BE"));
        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2022, 6, 17}, {0, 0}, QTimeZone("Europe/Berlin")))), QLatin1StringView("DE-BY"));
        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2022, 6, 18}, {0, 0}, QTimeZone("Europe/Berlin")))), QLatin1StringView("DE-BY"));
    }
};

QTEST_GUILESS_MAIN(TimelineSectionDelegateControllerTest)

#include "timelinesectiondelegatecontrollertest.moc"
