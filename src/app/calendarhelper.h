/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CALENDARHELPER_H
#define CALENDARHELPER_H

#include <KCalendarCore/Event>

class Transfer;

/** Helper functions for dealing with calendar data. */
namespace CalendarHelper
{

/** Extension of KItinerary::CalendarHandler::fillEvent for pre-Transfers. */
void fillPreTransfer(const KCalendarCore::Event::Ptr &event, const Transfer &transfer);

}

#endif // CALENDARHELPER_H
