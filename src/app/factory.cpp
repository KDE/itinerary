/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "factory.h"

#include <KItinerary/BoatTrip>
#include <KItinerary/Event>
#include <KItinerary/Person>
#include <KItinerary/Place>
#include <KItinerary/ProgramMembership>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>

QVariant Factory::makeBoatReservation()
{
    KItinerary::BoatReservation r;
    r.setReservationFor(KItinerary::BoatTrip());
    return r;
}

QVariant Factory::makeEventReservation()
{
    KItinerary::EventReservation r;
    r.setReservationFor(KItinerary::Event());
    return r;
}

QVariant Factory::makeFoodEstablishmentReservation()
{
    KItinerary::FoodEstablishmentReservation r;
    r.setReservationFor(KItinerary::FoodEstablishment());
    return r;
}

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

QVariant Factory::makeTicket()
{
    return KItinerary::Ticket();
}

#include "moc_factory.cpp"
