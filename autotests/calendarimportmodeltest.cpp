/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelper.h"

#include "calendarimportmodel.h"

#include <KItinerary/Event>
#include <KItinerary/Reservation>

#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/MemoryCalendar>

#include <QAbstractItemModelTester>
#include <QtTest/qtest.h>
#include <QSignalSpy>
#include <QStandardPaths>

using namespace KItinerary;

class CalendarImportModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
        QStandardPaths::setTestModeEnabled(true);
    }

    void testImport()
    {
        CalendarImportModel model;
        QAbstractItemModelTester modelTest(&model);

        KCalendarCore::MemoryCalendar::Ptr calendar(new KCalendarCore::MemoryCalendar(QTimeZone::utc()));
        KCalendarCore::ICalFormat format;
        QVERIFY(format.load(calendar, QLatin1StringView(SOURCE_DIR "/data/randa2017.ics")));

        QCOMPARE(model.hasSelection(), false);
        model.m_todayOverride = { 2017, 6, 27 };
        model.setCalendar(calendar.data());

        QCOMPARE(model.rowCount(), 5);
        QCOMPARE(model.hasSelection(), true);
        QCOMPARE(model.selectedReservations().size(), 3);

        auto idx = model.index(0, 0);
        QCOMPARE(idx.data(CalendarImportModel::TitleRole).toString(), QLatin1StringView("Hotel reservation: Haus Randa"));
        QCOMPARE(idx.data(CalendarImportModel::IconNameRole).toString(), QLatin1StringView("meeting-attending"));
        QCOMPARE(idx.data(CalendarImportModel::SelectedRole).toBool(), false);
        QVERIFY(model.setData(idx, true, CalendarImportModel::SelectedRole));

        idx = model.index(1, 0);
        QCOMPARE(idx.data(CalendarImportModel::TitleRole).toString(), QLatin1StringView("Train 241 from Visp to Randa"));
        QCOMPARE(idx.data(CalendarImportModel::IconNameRole).toString(), QLatin1StringView("qrc:///images/train.svg"));
        QCOMPARE(idx.data(CalendarImportModel::SelectedRole).toBool(), true);
        auto res = idx.data(CalendarImportModel::ReservationsRole)
                       .value<QList<QVariant>>();
        QCOMPARE(res.size(), 1);
        QVERIFY(JsonLd::isA<TrainReservation>(res.at(0)));
        QVERIFY(model.setData(idx, false, CalendarImportModel::SelectedRole));

        idx = model.index(2, 0);
        QCOMPARE(idx.data(CalendarImportModel::TitleRole).toString(), QLatin1StringView("KDE Randa Meeting 2017"));
        QCOMPARE(idx.data(CalendarImportModel::IconNameRole).toString(), QLatin1StringView("meeting-attending"));
        QCOMPARE(idx.data(CalendarImportModel::SelectedRole).toBool(), false);
        QVERIFY(model.setData(idx, true, CalendarImportModel::SelectedRole));

        idx = model.index(3, 0);
        QCOMPARE(idx.data(CalendarImportModel::TitleRole).toString(), QLatin1StringView("Restaurant reservation: Raclette"));
        QCOMPARE(idx.data(CalendarImportModel::IconNameRole).toString(), QLatin1StringView("qrc:///images/foodestablishment.svg"));
        QCOMPARE(idx.data(CalendarImportModel::SelectedRole).toBool(), true);
        res = idx.data(CalendarImportModel::ReservationsRole)
                  .value<QList<QVariant>>();
        QCOMPARE(res.size(), 1);
        QVERIFY(JsonLd::isA<FoodEstablishmentReservation>(res.at(0)));
        QVERIFY(model.setData(idx, false, CalendarImportModel::SelectedRole));

        idx = model.index(4, 0);
        QCOMPARE(idx.data(CalendarImportModel::TitleRole).toString(), QLatin1StringView("Randa -> Visp"));
        QCOMPARE(idx.data(CalendarImportModel::IconNameRole).toString(), QLatin1StringView("qrc:///images/train.svg"));
        QCOMPARE(idx.data(CalendarImportModel::SelectedRole).toBool(), true);
        res = idx.data(CalendarImportModel::ReservationsRole)
                  .value<QList<QVariant>>();
        QCOMPARE(res.size(), 1);
        QVERIFY(JsonLd::isA<TrainReservation>(res.at(0)));
        QVERIFY(model.setData(idx, false, CalendarImportModel::SelectedRole));

        QCOMPARE(model.hasSelection(), true);
        QCOMPARE(model.selectedReservations().size(), 2);

        auto ev = model.selectedReservations().at(0).value<KItinerary::Event>();
        QCOMPARE(ev.name(), QLatin1StringView("Hotel reservation: Haus Randa"));
        QVERIFY(ev.startDate().isValid());
        QVERIFY(ev.endDate().isValid());

        ev = model.selectedReservations().at(1).value<KItinerary::Event>();
        QCOMPARE(ev.name(), QLatin1StringView("KDE Randa Meeting 2017"));
        QVERIFY(ev.startDate().isValid());
        QVERIFY(ev.endDate().isValid());
    }
};

QTEST_GUILESS_MAIN(CalendarImportModelTest)

#include "calendarimportmodeltest.moc"
