/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendarhelper.h"
#include "transfer.h"

#include <KPublicTransport/Journey>

#include <KCalendarCore/ICalFormat>

#include <QStandardPaths>
#include <QtTest/qtest.h>

class CalendarHelperTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "UTC");
        QStandardPaths::setTestModeEnabled(true);
    }

    void testFillPreTransfer()
    {
        KCalendarCore::Event::Ptr event(new KCalendarCore::Event);
        event->setDtStart(QDateTime({2017, 9, 10}, {6, 30}, QTimeZone("Europe/Berlin")));

        Transfer transfer;
        transfer.setAlignment(Transfer::Before);
        transfer.setState(Transfer::Selected);
        transfer.setAnchorTimeDelta(60 * 60);
        transfer.setAnchorTime(QDateTime({2017, 9, 10}, {6, 30}, QTimeZone("Europe/Berlin")));
        transfer.setToName(QStringLiteral("Berlin Airport"));

        KPublicTransport::Journey jny;
        KPublicTransport::JourneySection section;
        section.setScheduledDepartureTime(QDateTime({2017, 9, 10}, {5, 0}, QTimeZone("Europe/Berlin")));
        section.setScheduledArrivalTime(QDateTime({2017, 9, 10}, {5, 30}, QTimeZone("Europe/Berlin")));
        jny.setSections({section});
        transfer.setJourney(jny);

        CalendarHelper::fillPreTransfer(event, transfer);
        KCalendarCore::ICalFormat format;
        qDebug().noquote() << format.toString(event);

        QCOMPARE(event->nonKDECustomProperty("X-APPLE-TRAVEL-DURATION"), QLatin1StringView("PT1H30M"));
        QCOMPARE(event->alarms().size(), 1);
        const auto alarm = event->alarms().at(0);
        QCOMPARE(alarm->startOffset().asSeconds(), -5700);
    }
};

QTEST_GUILESS_MAIN(CalendarHelperTest)

#include "calendarhelpertest.moc"
