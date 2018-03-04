/*
   Copyright (c) 2017-2018 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#include "boardingpass.h"
#include "pass_p.h"

using namespace KPkPass;

BoardingPass::BoardingPass(QObject *parent)
    : Pass(Pass::BoardingPass, parent)
{
}

BoardingPass::~BoardingPass() = default;

BoardingPass::TransitType BoardingPass::transitType() const
{
    const auto t = d->passData().value(QLatin1String("transitType")).toString();
    if (t == QLatin1String("PKTransitTypeAir")) {
        return Air;
    } else if (t == QLatin1String("PKTransitTypeBoat")) {
        return Boat;
    } else if (t == QLatin1String("PKTransitTypeBus")) {
        return Bus;
    } else if (t == QLatin1String("PKTransitTypeTrain")) {
        return Train;
    }
    return Generic;
}
