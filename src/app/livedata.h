/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LIVEDATA_H
#define LIVEDATA_H

#include <KPublicTransport/Journey>
#include <KPublicTransport/Stopover>

#include <QDateTime>
#include <QHash>

/** Realttime information about a public transport reservation */
class LiveData
{
public:
    enum Type { NoType = 0, Departure = 1, Arrival = 2, Journey = 4, AllTypes = Departure | Arrival | Journey };

    void setTimestamp(Type type, const QDateTime &dt);

    [[nodiscard]] bool isEmpty() const;

    KPublicTransport::Stopover departure;
    QDateTime departureTimestamp;
    KPublicTransport::Stopover arrival;
    QDateTime arrivalTimestamp;
    KPublicTransport::JourneySection journey;
    QDateTime journeyTimestamp;

    /** Load live data for reservation batch with id @resId. */
    [[nodiscard]] static LiveData load(const QString &resId);
    /** Store this data for reservation batch @p resId. */
    void store(const QString &resId, int types = AllTypes) const;
    /** Removes all stored data for a given id. */
    static void remove(const QString &resId);

    /** List all reservations for which there are stored live data information.
     *  Used for exporting.
     */
    [[nodiscard]] static std::vector<QString> listAll();

    /** Single JSON object representing @p ld, for use in exporting. */
    [[nodiscard]] static QJsonObject toJson(const LiveData &ld);
    /** Deserialize from JSON. */
    [[nodiscard]] static LiveData fromJson(const QJsonObject &obj);

    /** Clears all stored live data.
     *  @internal For unit tests only.
     */
    static void clearStorage();
};

#endif // LIVEDATA_H
