/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "factory.h"

#include <KItinerary/Place>
#include <KItinerary/Ticket>

QVariant Factory::makePlace()
{
    return KItinerary::Place();
}

QVariant Factory::makeTicket()
{
    return KItinerary::Ticket();
}
