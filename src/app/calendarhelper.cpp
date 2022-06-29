/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendarhelper.h"
#include "transfer.h"

#include <KCalendarCore/Alarm>
#include <KCalendarCore/ICalFormat>

#include <KLocalizedString>

#include <QDebug>

void CalendarHelper::fillPreTransfer(const KCalendarCore::Event::Ptr &event, const Transfer &transfer)
{
    if (transfer.state() != Transfer::Selected) {
        return;
    }

    // add a reminder 5min before the transfer departure
    const auto depDt = transfer.journey().hasExpectedDepartureTime() ? transfer.journey().expectedDepartureTime() : transfer.journey().scheduledDepartureTime();
    if (event->alarms().empty()) {
        KCalendarCore::Alarm::Ptr alarm(new KCalendarCore::Alarm(event.data()));
        KCalendarCore::Duration alarmDur(-depDt.secsTo(event->dtStart()) - 5 * 60);
        alarm->setStartOffset(alarmDur);
        alarm->setDisplayAlarm(i18n("Transfer to %1", transfer.toName()));
        alarm->setEnabled(true);
        event->addAlarm(alarm);
    }

    // add Apple travel duration property
    KCalendarCore::Duration travelDur(depDt.secsTo(event->dtStart()));
    KCalendarCore::ICalFormat format;
    event->setNonKDECustomProperty("X-APPLE-TRAVEL-DURATION", format.toString(travelDur), QStringLiteral("VALUE=DURATION"));

    // TODO add X-APPLE-TRAVEL-START property
}
