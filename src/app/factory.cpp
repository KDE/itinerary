/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "factory.h"

#include <KItinerary/Person>
#include <KItinerary/Place>
#include <KItinerary/ProgramMembership>
#include <KItinerary/Reservation>

QVariant Factory::makeLodgingReservation()
{
    KItinerary::LodgingReservation r;
    r.setReservationFor(KItinerary::LodgingBusiness());
    return r;
}

QVariant Factory::makePerson()
{
    return KItinerary::Person();
}

QVariant Factory::makePlace()
{
    return KItinerary::Place();
}

QVariant Factory::makeProgramMembership()
{
    return KItinerary::ProgramMembership();
}
