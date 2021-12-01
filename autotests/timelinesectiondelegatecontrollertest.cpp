/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include <locationhelper.h>
#include <timelinemodel.h>
#include <timelinesectiondelegatecontroller.h>
#include <applicationcontroller.h>
#include <reservationmanager.h>

#include <QUrl>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

using namespace KItinerary;

void initLocale()
{
    qputenv("TZ", "UTC");
    qputenv("LANG", "en_US");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

class TimelineSectionDelegateControllerTest : public QObject
{
    Q_OBJECT
private:
    QByteArray readFile(const QString &fn)
    {
        QFile f(fn);
        f.open(QFile::ReadOnly);
        return f.readAll();
    }

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testController()
    {
        ReservationManager mgr;
        Test::clearAll(&mgr);
        ApplicationController ctrl;
        ctrl.setReservationManager(&mgr);
        ctrl.importFromUrl(QUrl::fromLocalFile(QLatin1String(SOURCE_DIR "/../tests/randa2017.json")));

        TimelineModel model;
        model.setCurrentDateTime(QDateTime({2021, 3, 7}, {8, 0}));
        model.setReservationManager(&mgr);

        QCOMPARE(model.locationAtTime(QDateTime({2017, 9, 10}, {0, 0})), QVariant());
        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2017, 9, 11}, {0, 0}))), QLatin1String("CH-VS"));
        QCOMPARE(LocationHelper::regionCode(model.locationAtTime(QDateTime({2017, 9, 16}, {0, 0}))), QLatin1String("DE-BE"));

        TimelineSectionDelegateController controller;
        controller.setTimelineModel(&model);

        controller.setDateString(QStringLiteral("2021-03-07"));
        QCOMPARE(controller.isToday(), true);
        QCOMPARE(controller.title(), QLatin1String("Today"));
        QCOMPARE(controller.subTitle(), QString());
        QCOMPARE(controller.isHoliday(), false);

        controller.setDateString(QStringLiteral("2021-03-08"));
        QCOMPARE(controller.isToday(), false);
        QCOMPARE(controller.title(), QLatin1String("Monday, 3/8/21"));
        QCOMPARE(controller.subTitle(), QLatin1String("Internationaler Frauentag")); // DE-BE only, so proves the right region was selected
        QCOMPARE(controller.isHoliday(), true);

        model.setCurrentDateTime(QDateTime({2021, 12, 27}, {8, 0}));
        controller.setDateString(QStringLiteral("2021-12-31")); // named date, but not a bank holiday
        QCOMPARE(controller.isToday(), false);
        QCOMPARE(controller.subTitle(), QLatin1String("Silvester"));
        QCOMPARE(controller.isHoliday(), false);
        controller.setDateString(QStringLiteral("2022-01-01"));
        QCOMPARE(controller.isToday(), false);
        QCOMPARE(controller.subTitle(), QLatin1String("Neujahr"));
        QCOMPARE(controller.isHoliday(), true);
    }
};

QTEST_GUILESS_MAIN(TimelineSectionDelegateControllerTest)

#include "timelinesectiondelegatecontrollertest.moc"
