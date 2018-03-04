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

#ifndef KPKPASS_BOARDINGPASS_H
#define KPKPASS_BOARDINGPASS_H

#include "pass.h"

namespace KPkPass {

/** Boarding pass */
class KPKPASS_EXPORT BoardingPass : public Pass
{
    Q_GADGET
    Q_PROPERTY(TransitType transitType READ transitType)
public:
    enum TransitType {
        Air,
        Boat,
        Bus,
        Train,
        Generic
    };
    Q_ENUM(TransitType)

    explicit BoardingPass(QObject *parent = nullptr);
    ~BoardingPass() = default;

    TransitType transitType() const;
};

}

#endif // KPKPASS_BOARDINGPASS_H
