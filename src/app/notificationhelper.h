/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
