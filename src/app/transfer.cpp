/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "transfer.h"
#include "json.h"

#include <KItinerary/JsonLdDocument>

#include <QDebug>
#include <QJsonObject>

using namespace Qt::Literals::StringLiterals;

class TransferPrivate : public QSharedData
{
public:
    // ### any update here needs to also be reflected in the operator== implementation below
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

Transfer::Transfer(const Transfer &) = default;
Transfer::~Transfer() = default;

Transfer &Transfer::operator=(const Transfer &) = default;

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
    if (d->m_fromName.isEmpty()) {
        return d->m_from.name();
    }
    return d->m_fromName;
}

void Transfer::setFromName(const QString &fromName)
{
    d.detach();
    d->m_fromName = fromName;
}

QString Transfer::toName() const
{
    if (d->m_toName.isEmpty()) {
        return d->m_to.name();
    }
    return d->m_toName;
}

void Transfer::setToName(const QString &toName)
{
    d.detach();
    d->m_toName = toName;
}

bool Transfer::hasLocations() const
{
    return !d->m_from.isEmpty() && !d->m_to.isEmpty();
}

bool Transfer::hasCoordinates() const
{
    return d->m_from.hasCoordinate() && d->m_to.hasCoordinate();
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
            d->m_anchorDelta = (int)std::max(0ll, journey.scheduledArrivalTime().secsTo(d->m_anchorTime));
        } else {
            d->m_anchorDelta = (int)std::max(0ll, d->m_anchorTime.secsTo(journey.scheduledDepartureTime()));
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

void Transfer::setAnchorTime(const QDateTime &dt)
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

bool Transfer::isReachable() const
{
    if (state() == Transfer::Selected) {
        if (alignment() == Transfer::After) {
            const auto hasPT = std::any_of(journey().sections().begin(), journey().sections().end(), [](const auto &sec) {
                return sec.mode() == KPublicTransport::JourneySection::PublicTransport;
            });
            if (journey().hasExpectedDepartureTime()) {
                return !hasPT || journey().expectedDepartureTime() > anchorTime();
            }
            if (journey().scheduledDepartureTime().isValid()) {
                return !hasPT || journey().scheduledDepartureTime() > anchorTime();
            }
        }
    }
    return true;
}

QString Transfer::identifier() const
{
    return identifier(d->m_resId, d->m_alignment);
}

QString Transfer::identifier(const QString &resId, Transfer::Alignment alignment)
{
    return resId + (alignment == Transfer::Before ? "-BEFORE"_L1 : "-AFTER"_L1);
}

bool Transfer::operator==(const Transfer &other) const
{
    if (d->m_alignment == other.d->m_alignment && d->m_state == other.d->m_state && KPublicTransport::Location::isSame(d->m_from, other.d->m_from)
        && KPublicTransport::Location::isSame(d->m_to, other.d->m_to) && KPublicTransport::Journey::isSame(d->m_journey, other.d->m_journey)
        && d->m_fromName == other.d->m_fromName && d->m_toName == other.d->m_toName && d->m_anchorTime == other.d->m_anchorTime
        && d->m_anchorDelta == other.d->m_anchorDelta && d->m_floatingLocationType == other.d->m_floatingLocationType) {
        if (d->m_state == Transfer::Selected) {
            return false; // we lack a way to check if m_journey is exactly identical!
        }
        return true;
    }
    return false;
}

QJsonObject Transfer::toJson(const Transfer &transfer)
{
    auto obj = Json::toJson(transfer);
    obj.insert("from"_L1, KPublicTransport::Location::toJson(transfer.from()));
    obj.insert("to"_L1, KPublicTransport::Location::toJson(transfer.to()));
    obj.insert("journey"_L1, KPublicTransport::Journey::toJson(transfer.journey()));
    return obj;
}

Transfer Transfer::fromJson(const QJsonObject &obj)
{
    auto transfer = Json::fromJson<Transfer>(obj);
    transfer.setFrom(KPublicTransport::Location::fromJson(obj.value("from"_L1).toObject()));
    transfer.setTo(KPublicTransport::Location::fromJson(obj.value("to"_L1).toObject()));
    transfer.setJourney(KPublicTransport::Journey::fromJson(obj.value("journey"_L1).toObject()));

    if (transfer.state() == Searching) { // searching state cannot survive persisting
        transfer.setState(Pending);
    }

    return transfer;
}

#include "moc_transfer.cpp"
