/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#ifndef TRANSFER_H
#define TRANSFER_H

#include <KPublicTransport/Journey>
#include <KPublicTransport/Location>

#include <QExplicitlySharedDataPointer>
#include <QMetaType>

class TransferPrivate;

class QJsonObject;

/** Describes an actual or potential transfer between two locations.
 *  One of the locations is the "anchored" one, the other one the "floating" one.
 *  The anchored side is defining the time, and follows an reservation element it is anchored to.
 *  The floating side is either another reservation element, or a favorite location.
 */
class Transfer
{
    Q_GADGET
    Q_PROPERTY(Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(State state READ state WRITE setState)
    Q_PROPERTY(FloatingLocationType floatingLocationType READ floatingLocationType WRITE setFloatingLocationType)
    Q_PROPERTY(KPublicTransport::Location from READ from WRITE setFrom)
    Q_PROPERTY(KPublicTransport::Location to READ to WRITE setTo)
    Q_PROPERTY(QString fromName READ fromName WRITE setFromName)
    Q_PROPERTY(QString toName READ toName WRITE setToName)
    Q_PROPERTY(KPublicTransport::Journey journey READ journey WRITE setJourney)
    Q_PROPERTY(QString reservationId READ reservationId WRITE setReservationId)
    Q_PROPERTY(QDateTime anchorTime READ anchorTime WRITE setAnchorTime)
    Q_PROPERTY(int anchorTimeDelta READ anchorTimeDelta WRITE setAnchorTimeDelta)
    Q_PROPERTY(QDateTime journeyTime READ journeyTime STORED false)
public:
    Transfer();
    Transfer(const Transfer&);
    ~Transfer();
    Transfer& operator=(const Transfer&);

    /** Aligned to the begin or end of the corresponding reservation. */
    enum Alignment {
        Before,
        After
    };
    Q_ENUM(Alignment)
    Alignment alignment() const;
    void setAlignment(Alignment alignment);

    /** No journey selected, journey selected, or explicitly discarded. */
    enum State {
        UndefinedState,
        Pending,
        Selected,
        Discarded
    };
    Q_ENUM(State)
    State state() const;
    void setState(State state);

    /** Source of the floating side location. */
    enum FloatingLocationType {
        Reservation,
        FavoriteLocation
    };
    Q_ENUM(FloatingLocationType)
    FloatingLocationType floatingLocationType() const;
    void setFloatingLocationType(FloatingLocationType type);

    KPublicTransport::Location from() const;
    void setFrom(const KPublicTransport::Location &from);
    KPublicTransport::Location to() const;
    void setTo(const KPublicTransport::Location &to);
    /** From and to locations are sufficiently defined. */
    bool hasLocations() const;

    QString fromName() const;
    void setFromName(const QString &fromName);
    QString toName() const;
    void setToName(const QString &toName);

    KPublicTransport::Journey journey() const;
    void setJourney(const KPublicTransport::Journey &journey);

    QString reservationId() const;
    void setReservationId(const QString &resId);

    /** The time-wise fixed side of this transfer, ie. the start for Alignment::After and end for Alignment::Before. */
    QDateTime anchorTime() const;
    void setAnchorTime(const QDateTime &dt);

    /** The time offset in seconds to the anchor time the transfer should start/end. */
    int anchorTimeDelta() const;
    void setAnchorTimeDelta(int delta);

    /** Anchor time +/- anchor delta, ie. the time we actually want to arrive/depart. */
    QDateTime journeyTime() const;

    /** Unique identifier usable naming the file to store this into. */
    QString identifier() const;
    static QString identifier(const QString &resId, Transfer::Alignment alignment);

    static QJsonObject toJson(const Transfer &transfer);
    static Transfer fromJson(const QJsonObject &obj);

private:
    QExplicitlySharedDataPointer<TransferPrivate> d;
};

Q_DECLARE_METATYPE(Transfer)

#endif // TRANSFER_H
