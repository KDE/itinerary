/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

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

#ifndef KPUBLICTRANSPORT_JOURNEY_H
#define KPUBLICTRANSPORT_JOURNEY_H

#include "datatypes.h"
#include "line.h"
#include "location.h"

#include <QDateTime>

#include <vector>

namespace KPublicTransport {

class JourneySectionPrivate;

/** A segment of a journey plan. */
class JourneySection
{
    KPUBLICTRANSPORT_GADGET(JourneySection)
    /** Mode of transport for this section. */
    Q_PROPERTY(Mode mode READ mode WRITE setMode)

    /** Planned departure time. */
    Q_PROPERTY(QDateTime scheduledDepartureTime READ scheduledDepartureTime WRITE setScheduledDepartureTime)
    /** Actual departure time, if available.
     *  Set to invalid to indicate real-time data is not available.
     */
    Q_PROPERTY(QDateTime expectedDepartureTime READ expectedDepartureTime WRITE setExpectedDepartureTime)
    /** @c true if this has real-time data. */
    Q_PROPERTY(bool hasExpectedDepartureTime READ hasExpectedDepartureTime STORED false)
    /** Difference to schedule in minutes. */
    Q_PROPERTY(int departureDelay READ departureDelay STORED false)

    /** Planned arrival time. */
    Q_PROPERTY(QDateTime scheduledArrivalTime READ scheduledArrivalTime WRITE setScheduledArrivalTime)
    /** Actual arrival time, if available.
     *  Set to invalid to indicate real-time data is not available.
     */
    Q_PROPERTY(QDateTime expectedArrivalTime READ expectedArrivalTime WRITE setExpectedArrivalTime)
    /** @c true if this has real-time data. */
    Q_PROPERTY(bool hasExpectedArrivalTime READ hasExpectedArrivalTime STORED false)
    /** Difference to schedule in minutes. */
    Q_PROPERTY(int arrivalDelay READ arrivalDelay STORED false)

    /** Duration of the section in seconds. */
    Q_PROPERTY(int duration READ duration STORED false)

    /** Departure location of this segment. */
    Q_PROPERTY(KPublicTransport::Location from READ from WRITE setFrom)
    /** Arrival location of this segment. */
    Q_PROPERTY(KPublicTransport::Location to READ to WRITE setTo)
    /** Route to take on this segment. */
    Q_PROPERTY(KPublicTransport::Route route READ route WRITE setRoute)

    /** Planned departure platform. */
    Q_PROPERTY(QString scheduledDeparturePlatform READ scheduledDeparturePlatform WRITE setScheduledDeparturePlatform)
    /** Actual departure platform, in case real-time information are available. */
    Q_PROPERTY(QString expectedDeparturePlatform READ expectedDeparturePlatform WRITE setExpectedDeparturePlatform)
    /** @c true if real-time platform information are available. */
    Q_PROPERTY(bool hasExpectedDeparturePlatform READ hasExpectedDeparturePlatform STORED false)
    /** @c true if we have real-time platform information and the platform changed. */
    Q_PROPERTY(bool departurePlatformChanged READ departurePlatformChanged STORED false)

    /** Planned arrival platform. */
    Q_PROPERTY(QString scheduledArrivalPlatform READ scheduledArrivalPlatform WRITE setScheduledArrivalPlatform)
    /** Actual arrival platform, in case real-time information are available. */
    Q_PROPERTY(QString expectedArrivalPlatform READ expectedArrivalPlatform WRITE setExpectedArrivalPlatform)
    /** @c true if real-time platform information are available. */
    Q_PROPERTY(bool hasExpectedArrivalPlatform READ hasExpectedArrivalPlatform STORED false)
    /** @c true if we have real-time platform information and the platform changed. */
    Q_PROPERTY(bool arrivalPlatformChanged READ arrivalPlatformChanged STORED false)

public:
    /** Mode of transport. */
    enum Mode {
        Invalid,
        PublicTransport,
        Transfer,
        Walking,
        Waiting
    };
    Q_ENUM(Mode)
    Mode mode() const;
    void setMode(Mode mode);

    QDateTime scheduledDepartureTime() const;
    void setScheduledDepartureTime(const QDateTime &dt);
    QDateTime expectedDepartureTime() const;
    void setExpectedDepartureTime(const QDateTime &dt);
    bool hasExpectedDepartureTime() const;
    int departureDelay() const;

    QDateTime scheduledArrivalTime() const;
    void setScheduledArrivalTime(const QDateTime &dt);
    QDateTime expectedArrivalTime() const;
    void setExpectedArrivalTime(const QDateTime &dt);
    bool hasExpectedArrivalTime() const;
    int arrivalDelay() const;

    int duration() const;

    Location from() const;
    void setFrom(const Location &from);
    Location to() const;
    void setTo(const Location &to);
    Route route() const;
    void setRoute(const Route &route);

    QString scheduledDeparturePlatform() const;
    void setScheduledDeparturePlatform(const QString &platform);
    QString expectedDeparturePlatform() const;
    void setExpectedDeparturePlatform(const QString &platform);
    bool hasExpectedDeparturePlatform() const;
    bool departurePlatformChanged() const;

    QString scheduledArrivalPlatform() const;
    void setScheduledArrivalPlatform(const QString &platform);
    QString expectedArrivalPlatform() const;
    void setExpectedArrivalPlatform(const QString &platform);
    bool hasExpectedArrivalPlatform() const;
    bool arrivalPlatformChanged() const;
};

class JourneyPrivate;

/** A journey plan. */
class Journey
{
    KPUBLICTRANSPORT_GADGET(Journey)
    /** Journey sections for consumption by QML. */
    Q_PROPERTY(QVariantList sections READ sectionsVariant)
    /** Departure time of the journey, according to schedule. */
    Q_PROPERTY(QDateTime scheduledDepartureTime READ scheduledDepartureTime STORED false)
    /** Arrival time of the journey, according to schedule. */
    Q_PROPERTY(QDateTime scheduledArrivalTime READ scheduledArrivalTime STORED false)
    /** Duration of the entire journey in seconds. */
    Q_PROPERTY(int duration READ duration STORED false)
    /** Number of changes on this journey. */
    Q_PROPERTY(int numberOfChanges READ numberOfChanges STORED false)

public:
    /** The journey sections. */
    const std::vector<JourneySection>& sections() const;
    /** Moves the journey sections out of this object. */
    std::vector<JourneySection>&& takeSections();
    /** Sets the journey sections. */
    void setSections(std::vector<JourneySection> &&sections);

    QDateTime scheduledDepartureTime() const;
    QDateTime scheduledArrivalTime() const;
    int duration() const;
    int numberOfChanges() const;
private:
    QVariantList sectionsVariant() const;
};

}

Q_DECLARE_METATYPE(KPublicTransport::JourneySection)
Q_DECLARE_METATYPE(KPublicTransport::Journey)

#endif // KPUBLICTRANSPORT_JOURNEY_H
