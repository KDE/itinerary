/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tickettokenmodel.h"
#include "reservationmanager.h"

#include <KItinerary/Person>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>

#include <KLocalizedString>

#include <KUser>

using namespace KItinerary;

TicketTokenModel::TicketTokenModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

TicketTokenModel::~TicketTokenModel() = default;

ReservationManager *TicketTokenModel::reservationManager() const
{
    return m_resMgr;
}

void TicketTokenModel::setReservationManager(ReservationManager *mgr)
{
    m_resMgr = mgr;
    connect(m_resMgr, &ReservationManager::reservationRemoved, this, &TicketTokenModel::reservationRemoved);
    setReservationIds(m_pendingResIds);
}

QStringList TicketTokenModel::reservationIds() const
{
    return m_resIds;
}

void TicketTokenModel::setReservationIds(const QStringList &resIds)
{
    if (!m_resMgr) {
        m_pendingResIds = resIds;
        return;
    }

    beginResetModel();
    m_personNames.clear();
    m_personNames.reserve(m_resIds.size());
    QHash<QString, int> m_personIdx;

    for (const auto &resId : resIds) {
        const auto v = m_resMgr->reservation(resId);
        if (!JsonLd::canConvert<Reservation>(v)) {
            continue;
        }
        const auto res = JsonLd::convert<Reservation>(v);
        const auto ticket = res.reservedTicket().value<Ticket>();
        const auto person = JsonLd::convert<Reservation>(res).underName().value<Person>();
        if (ticket.ticketToken().isEmpty() && res.pkpassPassTypeIdentifier().isEmpty() && person.name().isEmpty() && ticket.name().isEmpty()) {
            continue;
        }

        m_resIds.push_back(resId);
        if (!person.name().isEmpty()) {
            m_personNames.push_back(person.name());
        } else {
            const auto idx = ++m_personIdx[res.reservedTicket().value<Ticket>().name()];
            if (JsonLd::isA<EventReservation>(v)) {
                m_personNames.push_back(i18n("Attendee %1", idx));
            } else {
                m_personNames.push_back(i18n("Traveler %1", idx));
            }
        }
    }

    endResetModel();
    Q_EMIT initialIndexChanged();
    m_pendingResIds.clear();
}

QVariant TicketTokenModel::reservationAt(int row) const
{
    if (!m_resMgr || row >= m_resIds.size() || row < 0) {
        return {};
    }
    return m_resMgr->reservation(m_resIds.at(row));
}

QString TicketTokenModel::reservationIdAt(int row) const
{
    if (!m_resMgr || row >= m_resIds.size() || row < 0) {
        return {};
    }
    return m_resIds.at(row);
}

int TicketTokenModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return (int)m_resIds.size();
}

QVariant TicketTokenModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_resMgr || index.row() >= m_resIds.size()) {
        return {};
    }

    auto res = m_resMgr->reservation(m_resIds.at(index.row()));
    switch (role) {
    case Qt::DisplayRole: {
        const auto ticket = JsonLd::convert<Reservation>(res).reservedTicket().value<Ticket>();
        if (index.row() < m_personNames.size()) {
            if (!ticket.name().isEmpty()) {
                return i18n("%1 (%2)", m_personNames.at(index.row()), ticket.name());
            }
            return m_personNames.at(index.row());
        }
        return ticket.name();
    }
    case ReservationRole:
        return res;
    }

    return {};
}

QHash<int, QByteArray> TicketTokenModel::roleNames() const
{
    auto names = QAbstractListModel::roleNames();
    names.insert(ReservationRole, "reservation");
    return names;
}

int TicketTokenModel::initialIndex() const
{
    if (!m_resMgr) {
        return 0;
    }

    const auto fullName = KUser().property(KUser::FullName).toString();
    qsizetype nameIdx = -1;
    qsizetype ticketIdx = -1;
    for (qsizetype i = 0; i < m_personNames.size(); ++i) {
        const auto nameMatch = m_personNames.at(i).compare(fullName, Qt::CaseInsensitive) == 0;
        if (nameMatch && nameIdx < 0) {
            nameIdx = i;
        }
        bool hasToken = false;
        const auto res = m_resMgr->reservation(m_resIds.at(i));
        if (JsonLd::canConvert<Reservation>(res)) {
            hasToken = !JsonLd::convert<Reservation>(res).reservedTicket().value<Ticket>().ticketToken().isEmpty();
        }
        if (hasToken && nameMatch) {
            return (int)i;
        }
        if (hasToken && ticketIdx < 0) {
            ticketIdx = i;
        }
    }
    return ticketIdx >= 0 ? (int)ticketIdx : std::max(0, (int)nameIdx);
}

void TicketTokenModel::reservationRemoved(const QString &resId)
{
    const auto idx = (int)m_resIds.indexOf(resId);
    if (idx < 0) {
        return;
    }

    beginRemoveRows({}, idx, idx);
    m_resIds.remove(idx);
    m_personNames.remove(idx);
    endRemoveRows();
}

#include "moc_tickettokenmodel.cpp"
