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
    if (mgr == m_resMgr) {
        return;
    }
    m_resMgr = mgr;
    connect(m_resMgr, &ReservationManager::batchChanged, this, &TicketTokenModel::batchChanged);
    connect(m_resMgr, &ReservationManager::batchContentChanged, this, &TicketTokenModel::batchChanged);
    connect(m_resMgr, &ReservationManager::reservationRemoved, this, &TicketTokenModel::reservationRemoved);
    reload();
}

QString TicketTokenModel::batchId() const
{
    return m_batchId;
}

void TicketTokenModel::setBatchId(const QString &batchId)
{
    if (m_batchId == batchId) {
        return;
    }
    m_batchId = batchId;
    Q_EMIT batchIdChanged();
    reload();
}

void TicketTokenModel::reload()
{
    if (!m_resMgr || m_batchId.isEmpty()) {
        return;
    }

    beginResetModel();
    const auto resIds = m_resMgr->reservationsForBatch(m_batchId);

    m_data.clear();
    m_data.reserve(resIds.size());
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

        Data d{ .resId = resId, .personName = person.name() };
        if (d.personName.isEmpty()) {
            const auto idx = ++m_personIdx[res.reservedTicket().value<Ticket>().name()];
            if (JsonLd::isA<EventReservation>(v)) {
                d.personName = i18n("Attendee %1", idx);
            } else {
                d.personName = i18n("Traveler %1", idx);
            }
        }
        m_data.push_back(std::move(d));
    }

    endResetModel();
    Q_EMIT initialIndexChanged();
    Q_EMIT selectionChanged();
}

QVariant TicketTokenModel::reservationAt(int row) const
{
    if (!m_resMgr || row >= (int)m_data.size() || row < 0) {
        return {};
    }
    return m_resMgr->reservation(m_data[row].resId);
}

QString TicketTokenModel::reservationIdAt(int row) const
{
    if (!m_resMgr || row >= (int)m_data.size() || row < 0) {
        return {};
    }
    return m_data[row].resId;
}

int TicketTokenModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return (int)m_data.size();
}

QVariant TicketTokenModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_resMgr || index.row() >= (int)m_data.size()) {
        return {};
    }

    const auto &d = m_data[index.row()];
    auto res = m_resMgr->reservation(d.resId);
    switch (role) {
    case Qt::DisplayRole: {
        const auto ticket = JsonLd::convert<Reservation>(res).reservedTicket().value<Ticket>();
        if (!ticket.name().isEmpty() && !d.personName.isEmpty()) {
            return i18n("%1 (%2)", d.personName, ticket.name());
        }
        return d.personName.isEmpty() ? ticket.name() : d.personName;
    }
    case ReservationRole:
        return res;
    case SelectedRole:
        return d.selected;
    }

    return {};
}

bool TicketTokenModel::setData(const QModelIndex &index, const QVariant &data, int role)
{
    if (!index.isValid() || index.row() >= (int)m_data.size() || role != SelectedRole) {
        return false;
    }

    auto &d = m_data[index.row()];
    d.selected = data.toBool();
    Q_EMIT dataChanged(index, index);
    Q_EMIT selectionChanged();
    return true;
}

QHash<int, QByteArray> TicketTokenModel::roleNames() const
{
    auto names = QAbstractListModel::roleNames();
    names.insert(ReservationRole, "reservation");
    names.insert(SelectedRole, "selected");
    return names;
}

int TicketTokenModel::initialIndex() const
{
    if (!m_resMgr) {
        return 0;
    }

    const auto fullName = KUser().property(KUser::FullName).toString();
    int nameIdx = -1;
    int ticketIdx = -1;
    for (int i = 0; i < (int)m_data.size(); ++i) {
        const auto &d = m_data[i];
        const auto nameMatch = d.personName.compare(fullName, Qt::CaseInsensitive) == 0;
        if (nameMatch && nameIdx < 0) {
            nameIdx = i;
        }
        bool hasToken = false;
        const auto res = m_resMgr->reservation(d.resId);
        if (JsonLd::canConvert<Reservation>(res)) {
            hasToken = !JsonLd::convert<Reservation>(res).reservedTicket().value<Ticket>().ticketToken().isEmpty();
        }
        if (hasToken && nameMatch) {
            return i;
        }
        if (hasToken && ticketIdx < 0) {
            ticketIdx = i;
        }
    }
    return ticketIdx >= 0 ? ticketIdx : std::max(0, nameIdx);
}

QStringList TicketTokenModel::selectedReservationIds() const
{
    QStringList l;
    for (const auto &d :m_data) {
        if (d.selected) {
            l.push_back(d.resId);
        }
    }
    return l;
}

void TicketTokenModel::batchChanged(const QString &batchId)
{
    if (m_batchId != batchId) {
        return;
    }

    reload(); // could probably be done incrementally and with preserving selection
}

void TicketTokenModel::reservationRemoved(const QString &resId)
{
    const auto it = std::ranges::find_if(m_data, [&resId](const auto &data) {
        return data.resId == resId;
    });
    if (it == std::end(m_data)) {
        return;
    }

    const auto idx = (int)std::distance(std::begin(m_data), it);
    beginRemoveRows({}, idx, idx);
    m_data.erase(it);
    endRemoveRows();
}

#include "moc_tickettokenmodel.cpp"
