/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    Q_PROPERTY(bool isReachable READ isReachable STORED false)
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
    [[nodiscard]] Alignment alignment() const;
    void setAlignment(Alignment alignment);

    /** No journey selected, journey selected, or explicitly discarded. */
    enum State {
        UndefinedState,
        Pending,
        Selected,
        Discarded,
        Searching
    };
    Q_ENUM(State)
    [[nodiscard]] State state() const;
    void setState(State state);

    /** Source of the floating side location. */
    enum FloatingLocationType {
        Reservation,
        FavoriteLocation
    };
    Q_ENUM(FloatingLocationType)
    [[nodiscard]] FloatingLocationType floatingLocationType() const;
    void setFloatingLocationType(FloatingLocationType type);

    [[nodiscard]] KPublicTransport::Location from() const;
    void setFrom(const KPublicTransport::Location &from);
    [[nodiscard]] KPublicTransport::Location to() const;
    void setTo(const KPublicTransport::Location &to);
    /** From and to locations are sufficiently defined. */
    [[nodiscard]] bool hasLocations() const;
    /** From and to locations have geo coordinates.
     *  This imples hasLocations() but is stricter than that.
     *  Used for automatically adding transfers.
     */
    [[nodiscard]] bool hasCoordinates() const;

    [[nodiscard]] QString fromName() const;
    void setFromName(const QString &fromName);
    [[nodiscard]] QString toName() const;
    void setToName(const QString &toName);

    [[nodiscard]] KPublicTransport::Journey journey() const;
    void setJourney(const KPublicTransport::Journey &journey);

    [[nodiscard]] QString reservationId() const;
    void setReservationId(const QString &resId);

    /** The time-wise fixed side of this transfer, ie. the start for Alignment::After and end for Alignment::Before. */
    [[nodiscard]] QDateTime anchorTime() const;
    void setAnchorTime(const QDateTime &dt);

    /** The time offset in seconds to the anchor time the transfer should start/end. */
    [[nodiscard]] int anchorTimeDelta() const;
    void setAnchorTimeDelta(int delta);

    /** Anchor time +/- anchor delta, ie. the time we actually want to arrive/depart. */
    [[nodiscard]] QDateTime journeyTime() const;

    /** Returns @c false when this transfer isn't reachable (e.g. due to anchor time changes or a wrong journey pick). */
    [[nodiscard]] bool isReachable() const;

    /** Unique identifier usable naming the file to store this into. */
    [[nodiscard]] QString identifier() const;
    [[nodiscard]] static QString identifier(const QString &resId, Transfer::Alignment alignment);

    /** Check if transfers are considered identical
     *  in the sense that no update is required.
     *  This errs on the safe side and can report false even if there is actually no change,
     *  as it cannot check for identical journeys.
     */
    [[nodiscard]] bool operator==(const Transfer &other) const;

    [[nodiscard]] static QJsonObject toJson(const Transfer &transfer);
    [[nodiscard]] static Transfer fromJson(const QJsonObject &obj);

private:
    QExplicitlySharedDataPointer<TransferPrivate> d;
};

Q_DECLARE_METATYPE(Transfer)

#endif // TRANSFER_H
