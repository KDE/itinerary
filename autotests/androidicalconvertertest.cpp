/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../src/calendarextras/androidicalconverter.cpp"
#include "../src/calendarextras/android/eventdata.h"

#include <QObject>
#include <QTest>

using namespace Qt::Literals::StringLiterals;
using namespace KAndroidExtras;

class AndroidIcalConverterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testEvent()
    {
        JniEventData data;
        data.title = QStringLiteral("summary");
        data.allDay = false;
        data.dtStart = 1653055380000;
        data.startTimezone = QStringLiteral("Europe/Brussels");

        auto event = AndroidIcalConverter::readEvent(data);
        QCOMPARE(event->summary(), QLatin1StringView("summary"));
        QCOMPARE(event->allDay(), false);
        QCOMPARE(event->dtStart(), QDateTime({2022, 5, 20}, {16, 3}, QTimeZone("Europe/Brussels")));

        const auto out = AndroidIcalConverter::writeEvent(event);
        QCOMPARE(out.title, QLatin1StringView("summary"));
        QCOMPARE(out.allDay, false);
        QCOMPARE(out.dtStart, 1653055380000);
        QCOMPARE(out.startTimezone, QLatin1StringView("Europe/Brussels"));
    }

    void testAllDayEvent()
    {
        JniEventData data;
        data.title = u"summary"_s;
        data.allDay = true;
        data.dtStart = 1718755200000;
        data.startTimezone = u"UTC"_s;
        data.dtEnd = 1718841600000;
        data.endTimezone = u"UTC"_s;

        auto event = AndroidIcalConverter::readEvent(data);
        QCOMPARE(event->summary(), "summary"_L1);
        QCOMPARE(event->allDay(), true);
        QCOMPARE(event->dtStart().date(), QDate(2024, 6, 19));
        QCOMPARE(event->dtEnd().date(), QDate(2024, 6, 19)); // non-inclusive

        const auto out = AndroidIcalConverter::writeEvent(event);
        QCOMPARE(out.title, "summary"_L1);
        QCOMPARE(out.allDay, true);
        QCOMPARE(out.dtStart, 1718755200000);
        QCOMPARE(out.startTimezone, "UTC"_L1);
        QCOMPARE(out.dtEnd, 1718841600000);
        QCOMPARE(out.endTimezone, "UTC"_L1);
    }

    void testAlarm()
    {
        KCalendarCore::Event ev;
        auto data = Jni::fromHandle<JniReminderData>(QJniObject{});
        data.minutes = 5;
        auto alarm = AndroidIcalConverter::readAlarm(data, &ev);
        QCOMPARE(alarm->startOffset().asSeconds(), -300);

        const auto out = AndroidIcalConverter::writeAlarm(alarm);
        QCOMPARE(out.minutes, 5);
    }

    void testAttendee()
    {
        auto data = Jni::fromHandle<JniAttendeeData>(QJniObject{});
        data.name = QStringLiteral("Dr Konqi");
        data.email = QStringLiteral("null@kde.org");
        auto attendee = AndroidIcalConverter::readAttendee(data);
        QCOMPARE(attendee.fullName(), QLatin1StringView("Dr Konqi <null@kde.org>"));

        const auto out = AndroidIcalConverter::writeAttendee(attendee);
        QCOMPARE(out.name, QLatin1StringView("Dr Konqi"));
        QCOMPARE(out.email, QLatin1StringView("null@kde.org"));
    }

    void testExtendedProperties()
    {
        KCalendarCore::Event ev;
        AndroidIcalConverter::addExtendedProperty(&ev, QStringLiteral("CREATED"), QStringLiteral("20211116T193700Z"));
        AndroidIcalConverter::addExtendedProperty(&ev, QStringLiteral("GEO"), QStringLiteral("52.525;13.369"));
        AndroidIcalConverter::addExtendedProperty(&ev, QStringLiteral("X-KDE-KITINERARY-RESERVATION"), QStringLiteral("<json>"));

        QCOMPARE(ev.customProperty("KITINERARY", "RESERVATION"), QLatin1StringView("<json>"));
        QVERIFY(ev.hasGeo());
        QCOMPARE(ev.geoLatitude(), 52.525f);
        QCOMPARE(ev.geoLongitude(), 13.369f);
        QCOMPARE(ev.created(), QDateTime({2021, 11, 16}, {19, 37}, QTimeZone::UTC));

        const auto out = AndroidIcalConverter::writeExtendedProperties(&ev);
        QCOMPARE(out.size(), 3);
        QCOMPARE(out[0].name, QLatin1StringView("vnd.android.cursor.item/vnd.ical4android.unknown-property"));
        QCOMPARE(QString(out[0].value), QLatin1StringView("[\"CREATED\",\"20211116T193700Z\"]"));
        QCOMPARE(QString(out[1].value), QLatin1StringView("[\"GEO\",\"52.525002;13.369000\"]"));
        QCOMPARE(QString(out[2].value), QLatin1StringView("[\"X-KDE-KITINERARY-RESERVATION\",\"<json>\"]"));
    }

    void testReadRDate()
    {
        // date/time variants
        QCOMPARE(AndroidIcalConverter::readRDates<QDateTime>(QString()), QList<QDateTime>());
        QCOMPARE(AndroidIcalConverter::readRDates<QDateTime>(QStringLiteral("20220520T200000Z")),
                 QList<QDateTime>({QDateTime({2022, 5, 20}, {20, 0}, QTimeZone::UTC)}));
        QCOMPARE(AndroidIcalConverter::readRDates<QDateTime>(QStringLiteral("Europe/Brussels;20211224T153000,20211231T153000,20220107T153000")),
                 QList<QDateTime>({QDateTime({2021, 12, 24}, {15, 30}, QTimeZone("Europe/Brussels")),
                                   QDateTime({2021, 12, 31}, {15, 30}, QTimeZone("Europe/Brussels")),
                                   QDateTime({2022, 1, 7}, {15, 30}, QTimeZone("Europe/Brussels"))}));
        QCOMPARE(AndroidIcalConverter::readRDates<QDateTime>(QStringLiteral("20201230T000000Z,20211229T190000Z")),
                 QList<QDateTime>({QDateTime({2020, 12, 30}, {0, 0}, QTimeZone::UTC), QDateTime({2021, 12, 29}, {19, 0}, QTimeZone::UTC)}));
        QCOMPARE(AndroidIcalConverter::readRDates<QDateTime>(QStringLiteral("Europe/Helsinki;20210513T122346")),
                 QList<QDateTime>({QDateTime({2021, 05, 13}, {12, 23, 46}, QTimeZone("Europe/Helsinki"))}));

        // TODO date-only variants
    }

    void testWriteRDate()
    {
        QCOMPARE(AndroidIcalConverter::writeRDates(QList<QDateTime>()), QString());
        QCOMPARE(AndroidIcalConverter::writeRDates(QList<QDateTime>({QDateTime({2022, 5, 20}, {20, 0}, QTimeZone::UTC)})),
                 QLatin1StringView("20220520T200000Z"));
        QCOMPARE(AndroidIcalConverter::writeRDates(QList<QDateTime>({QDateTime({2021, 05, 13}, {12, 23, 46}, QTimeZone("Europe/Helsinki"))})),
                 QLatin1StringView("Europe/Helsinki;20210513T122346"));
        QCOMPARE(AndroidIcalConverter::writeRDates(
                     QList<QDateTime>({QDateTime({2020, 12, 30}, {0, 0}, QTimeZone::UTC), QDateTime({2021, 12, 29}, {19, 0}, QTimeZone::UTC)})),
                 QLatin1StringView("20201230T000000Z,20211229T190000Z"));
        QCOMPARE(AndroidIcalConverter::writeRDates(QList<QDateTime>({QDateTime({2021, 12, 24}, {15, 30}, QTimeZone("Europe/Brussels")),
                                                                     QDateTime({2021, 12, 31}, {15, 30}, QTimeZone::UTC),
                                                                     QDateTime({2022, 1, 7}, {15, 30}, QTimeZone("Europe/Brussels"))})),
                 QLatin1StringView("Europe/Brussels;20211224T153000,20211231T163000,20220107T153000"));

        QCOMPARE(AndroidIcalConverter::writeRDates(QList<QDate>()), QString());
        QCOMPARE(AndroidIcalConverter::writeRDates(QList<QDate>({{2022, 5, 28}})), QLatin1StringView("20220528"));
        QCOMPARE(AndroidIcalConverter::writeRDates(QList<QDate>({{2022, 5, 28}, {2022, 5, 29}})), QLatin1StringView("20220528,20220529"));
    }
};

QTEST_APPLESS_MAIN(AndroidIcalConverterTest)

#include "androidicalconvertertest.moc"
