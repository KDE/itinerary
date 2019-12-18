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
    QString m_resId;
    QDateTime m_anchorTime;
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

QJsonObject Transfer::toJson(const Transfer &transfer)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("alignment"), transfer.alignment()); // TODO store the enum properly
    obj.insert(QStringLiteral("state"), transfer.state()); // TODO store the enum properly
    obj.insert(QStringLiteral("from"), KPublicTransport::Location::toJson(transfer.from()));
    obj.insert(QStringLiteral("to"), KPublicTransport::Location::toJson(transfer.to()));
    obj.insert(QStringLiteral("journey"), KPublicTransport::Journey::toJson(transfer.journey()));
    obj.insert(QStringLiteral("reservationId"), transfer.reservationId());
    obj.insert(QStringLiteral("anchorTime"), KItinerary::JsonLdDocument::toJson(transfer.anchorTime()));
    return obj;
}

Transfer Transfer::fromJson(const QJsonObject &obj)
{
    Transfer transfer;
    transfer.setAlignment(static_cast<Transfer::Alignment>(obj.value(QLatin1String("alignment")).toInt()));
    transfer.setState(static_cast<Transfer::State>(obj.value(QLatin1String("state")).toInt()));
    transfer.setFrom(KPublicTransport::Location::fromJson(obj.value(QLatin1String("from")).toObject()));
    transfer.setTo(KPublicTransport::Location::fromJson(obj.value(QLatin1String("to")).toObject()));
    transfer.setJourney(KPublicTransport::Journey::fromJson(obj.value(QLatin1String("journey")).toObject()));
    transfer.setReservationId(obj.value(QLatin1String("reservationId")).toString());
    transfer.setAnchorTime(KItinerary::JsonLdDocument::fromJson(obj.value(QLatin1String("anchorTime")).toObject()).toDateTime());
    return transfer;
}
