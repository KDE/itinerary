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

#ifndef LIVEDATA_H
#define LIVEDATA_H

#include <KPublicTransport/Journey>
#include <KPublicTransport/Stopover>

#include <QDateTime>
#include <QHash>

class KNotification;

/** Realttime information about a public transport reservation */
class LiveData
{
public:
    enum Type {
        NoType = 0,
        Departure = 1,
        Arrival = 2,
        Journey = 4,
        AllTypes = Departure | Arrival | Journey
    };

    KPublicTransport::Stopover departure;
    QDateTime departureTimestamp;
    KPublicTransport::Stopover arrival;
    QDateTime arrivalTimestamp;
    KPublicTransport::JourneySection journey;
    QDateTime journeyTimestamp;

    KNotification *notification = nullptr;

    /** Load live data for reservation batch with id @resId. */
    static LiveData load(const QString &resId);
    /** Store this data for reservation batch @p resId. */
    void store(const QString &resId, int types = AllTypes) const;

    /** List all reservations for which there are stored live data information.
     *  Used for exporting.
     */
    static std::vector<QString> listAll();

    /** Clears all stored live data.
     *  @internal For unit tests only.
     */
    static void clearStorage();
};

#endif // LIVEDATA_H
