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
    enum Type { NoType = 0, Departure = 1, Arrival = 2, Journey = 4 };

    [[nodiscard]] bool isEmpty() const;

    KPublicTransport::JourneySection trip;
    QDateTime journeyTimestamp;
    qsizetype departureIndex = -1;
    qsizetype arrivalIndex = -1;

    [[nodiscard]] KPublicTransport::JourneySection journey() const;
    [[nodiscard]] KPublicTransport::Stopover departure() const;
    [[nodiscard]] KPublicTransport::Stopover arrival() const;

    /** Load live data for reservation batch with id @resId. */
    [[nodiscard]] static LiveData load(const QString &resId);
    /** Store this data for reservation batch @p resId. */
    void store(const QString &resId) const;
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

    /** Base path for live data storage.
     *  Do not use directly, only exposed for data migration.
     */
    [[nodiscard]] static QString basePath();

    /** Clears all stored live data.
     *  @internal For unit tests only.
     */
    static void clearStorage();

private:
    /** Populate departure/arrival indexes for legacy data. */
    void recoverIndexes();
};

#endif // LIVEDATA_H
