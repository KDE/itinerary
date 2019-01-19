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

#include "tickettokenmodel.h"
#include "reservationmanager.h"

#include <KItinerary/Person>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>

#include <KLocalizedString>

using namespace KItinerary;

TicketTokenModel::TicketTokenModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

TicketTokenModel::~TicketTokenModel() = default;

QObject * TicketTokenModel::reservationManager() const
{
    return m_resMgr;
}

void TicketTokenModel::setReservationManager(QObject *mgr)
{
    m_resMgr = qobject_cast<ReservationManager*>(mgr);
    setReservationIds(m_pendingResIds);
}

QStringList TicketTokenModel::reservationIds() const
{
    return m_resIds;
}

void TicketTokenModel::setReservationIds(const QStringList& resIds)
{
    if (!m_resMgr) {
        m_pendingResIds = resIds;
        return;
    } else {
        m_pendingResIds.clear();
    }

    beginResetModel();
    for (const auto &resId : resIds) {
        const auto v = m_resMgr->reservation(resId);
        if (!JsonLd::canConvert<Reservation>(v))
            continue;
        const auto res = JsonLd::convert<Reservation>(v);
        const auto ticket = res.reservedTicket().value<Ticket>();
        if (!ticket.ticketToken().isEmpty() || !res.pkpassPassTypeIdentifier().isEmpty()) {
            m_resIds.push_back(resId);
        }
    }
    endResetModel();
}

QVariant TicketTokenModel::reservationAt(int row) const
{
    if (!m_resMgr || row >= m_resIds.size() || row < 0)
        return {};
    return m_resMgr->reservation(m_resIds.at(row));
}

QString TicketTokenModel::reservationIdAt(int row) const
{
    if (!m_resMgr || row >= m_resIds.size() || row < 0)
        return {};
    return m_resIds.at(row);
}

int TicketTokenModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_resIds.size();
}

QVariant TicketTokenModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_resMgr || index.row() >= m_resIds.size())
        return {};

    const auto res = m_resMgr->reservation(m_resIds.at(index.row()));
    switch (role) {
        case Qt::DisplayRole:
        {
            const auto person = JsonLd::convert<Reservation>(res).underName().value<Person>();
            if (!person.name().isEmpty())
                return person.name();
            return i18n("Traveler %1", index.row() + 1);
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
