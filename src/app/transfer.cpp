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

#include "transfer.h"
#include "json.h"

#include <KItinerary/JsonLdDocument>

#include <QDebug>
#include <QJsonObject>

class TransferPrivate : public QSharedData
{
public:
    Transfer::Alignment m_alignment = Transfer::Before;
    Transfer::State m_state = Transfer::UndefinedState;
    KPublicTransport::Location m_from;
    KPublicTransport::Location m_to;
    KPublicTransport::Journey m_journey;
    QString m_fromName;
    QString m_toName;
    QString m_resId;
    QDateTime m_anchorTime;
    int m_anchorDelta = 0;
    Transfer::FloatingLocationType m_floatingLocationType = Transfer::Reservation;
};

Transfer::Transfer()
    : d(new TransferPrivate)
{
}

Transfer::Transfer(const Transfer&) = default;
Transfer::~Transfer() = default;

Transfer& Transfer::operator=(const Transfer&) = default;

Transfer::Alignment Transfer::alignment() const
{
    return d->m_alignment;
}

void Transfer::setAlignment(Transfer::Alignment alignment)
{
    d.detach();
    d->m_alignment = alignment;
}

Transfer::State Transfer::state() const
{
    return d->m_state;
}

void Transfer::setState(Transfer::State state)
{
    d.detach();
    d->m_state = state;
}

Transfer::FloatingLocationType Transfer::floatingLocationType() const
{
    return d->m_floatingLocationType;
}

void Transfer::setFloatingLocationType(Transfer::FloatingLocationType type)
{
    d.detach();
    d->m_floatingLocationType = type;
}

KPublicTransport::Location Transfer::from() const
{
    return d->m_from;
}

void Transfer::setFrom(const KPublicTransport::Location &from)
{
    d.detach();
    d->m_from = from;
}

KPublicTransport::Location Transfer::to() const
{
    return d->m_to;
}

void Transfer::setTo(const KPublicTransport::Location &to)
{
    d.detach();
    d->m_to = to;
}

QString Transfer::fromName() const
{
    if (d->m_fromName.isEmpty())
        return d->m_from.name();
    return d->m_fromName;
}

void Transfer::setFromName(const QString &fromName)
{
    d.detach();
    d->m_fromName = fromName;
}

QString Transfer::toName() const
{
    if (d->m_toName.isEmpty())
        return d->m_to.name();
    return d->m_toName;
}

void Transfer::setToName(const QString& toName)
{
    d.detach();
    d->m_toName = toName;
}

bool Transfer::hasLocations() const
{
    return !d->m_from.isEmpty() && !d->m_to.isEmpty();
}

KPublicTransport::Journey Transfer::journey() const
{
    return d->m_journey;
}

void Transfer::setJourney(const KPublicTransport::Journey &journey)
{
    d.detach();
    d->m_journey = journey;
    if (journey.scheduledArrivalTime().isValid()) {
        if (d->m_alignment == Transfer::Before) {
            d->m_anchorDelta = std::max(0ll, journey.scheduledArrivalTime().secsTo(d->m_anchorTime));
        } else {
            d->m_anchorDelta = std::max(0ll, d->m_anchorTime.secsTo(journey.scheduledDepartureTime()));
        }
    }
}

QString Transfer::reservationId() const
{
    return d->m_resId;
}

void Transfer::setReservationId(const QString &resId)
{
    d.detach();
    d->m_resId = resId;
}

QDateTime Transfer::anchorTime() const
{
    return d->m_anchorTime;
}

void Transfer::setAnchorTime(const QDateTime& dt)
{
    d.detach();
    d->m_anchorTime = dt;
}

int Transfer::anchorTimeDelta() const
{
    return d->m_anchorDelta;
}

void Transfer::setAnchorTimeDelta(int delta)
{
    d.detach();
    d->m_anchorDelta = delta;
}

QDateTime Transfer::journeyTime() const
{
    auto dt = anchorTime();
    dt = dt.addSecs(alignment() == Transfer::Before ? -anchorTimeDelta() : anchorTimeDelta());
    return std::max(dt, QDateTime::currentDateTime());
}

QString Transfer::identifier() const
{
    return identifier(d->m_resId, d->m_alignment);
}

QString Transfer::identifier(const QString &resId, Transfer::Alignment alignment)
{
    return resId + (alignment == Transfer::Before ? QLatin1String("-BEFORE") : QLatin1String("-AFTER"));
}

QJsonObject Transfer::toJson(const Transfer &transfer)
{
    auto obj = Json::toJson(transfer);
    obj.insert(QStringLiteral("from"), KPublicTransport::Location::toJson(transfer.from()));
    obj.insert(QStringLiteral("to"), KPublicTransport::Location::toJson(transfer.to()));
    obj.insert(QStringLiteral("journey"), KPublicTransport::Journey::toJson(transfer.journey()));
    return obj;
}

Transfer Transfer::fromJson(const QJsonObject &obj)
{
    auto transfer = Json::fromJson<Transfer>(obj);
    transfer.setFrom(KPublicTransport::Location::fromJson(obj.value(QLatin1String("from")).toObject()));
    transfer.setTo(KPublicTransport::Location::fromJson(obj.value(QLatin1String("to")).toObject()));
    transfer.setJourney(KPublicTransport::Journey::fromJson(obj.value(QLatin1String("journey")).toObject()));
    return transfer;
}
