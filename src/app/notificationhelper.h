/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef NOTIFICATIONHELPER_H
#define NOTIFICATIONHELPER_H

#include "livedata.h"

namespace KPublicTransport {
class Stopover;
}

/** Helper methods for generating notifications about itinerary changes. */
namespace NotificationHelper
{
    /** Checks whether something relevant changed and we should notify about this. */
    bool shouldNotify(const KPublicTransport::Stopover &oldStop, const KPublicTransport::Stopover &newStop, LiveData::Type context);

    /** Returns the title string for the disruption notification for @p data. */
    QString title(const LiveData &data);
    /** Returns the message text for the disruption notification for @p data. */
    QString message(const LiveData &data);
}

#endif // NOTIFICATIONHELPER_H
