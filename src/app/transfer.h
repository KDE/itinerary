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

#include <QExplicitlySharedDataPointer>
#include <QMetaType>

class TransferPrivate;

class QJsonObject;

/** Describes an actual or potential transfer between two reservation elements. */
class Transfer
{
    Q_GADGET
    Q_PROPERTY(Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(State state READ state WRITE setState)
    Q_PROPERTY(KPublicTransport::Journey journey READ journey WRITE setJourney)
    Q_PROPERTY(QString reservationId READ reservationId WRITE setReservationId)
    Q_PROPERTY(QDateTime anchorTime READ anchorTime WRITE setAnchorTime)
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
        Valid,
        Discarded
    };
    Q_ENUM(State)
    State state() const;
    void setState(State state);

    KPublicTransport::Journey journey() const;
    void setJourney(const KPublicTransport::Journey &journey);

    QString reservationId() const;
    void setReservationId(const QString &resId);

    /** The time-wise fixed side of this transfer, ie. the start for Alignment::After and end for Alignment::Before. */
    QDateTime anchorTime() const;
    void setAnchorTime(const QDateTime &dt);

    static QJsonObject toJson(const Transfer &transfer);
    static Transfer fromJson(const QJsonObject &obj);

private:
    QExplicitlySharedDataPointer<TransferPrivate> d;
};

Q_DECLARE_METATYPE(Transfer)

#endif // TRANSFER_H
