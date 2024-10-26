// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "tripgrouplocationmodel.h"

#include "publictransport.h"
#include "reservationmanager.h"
#include "tripgroup.h"

#include <KItinerary/LocationUtil>
#include <KItinerary/SortUtil>

using namespace KItinerary;

TripGroupLocationModel::TripGroupLocationModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &TripGroupLocationModel::setupChanged, this, &TripGroupLocationModel::populate);
}

TripGroupLocationModel::~TripGroupLocationModel() = default;

void TripGroupLocationModel::setTripGroupManager(TripGroupManager *tripGroupMgr)
{
    if (m_tripGroupMgr == tripGroupMgr) {
        return;
    }

    m_tripGroupMgr = tripGroupMgr;
    connect(m_tripGroupMgr, &TripGroupManager::tripGroupChanged, this, [this](const QString &tgId) {
        if (m_tripGroupId == tgId) {
            populate();
        }
    });
    Q_EMIT setupChanged();
}

int TripGroupLocationModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : (int)m_locations.size();
}

QVariant TripGroupLocationModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    const auto &entry = m_locations[index.row()];
    switch (role) {
    case LocationRole:
        return entry.location;
    case LocationNameRole:
        return entry.location.name();
    case LastUsedRole:
        return entry.lastUse;
    case UseCountRole:
        return entry.useCount;
    case IsRemovableRole:
        return false;
    }
    return {};
}

QHash<int, QByteArray> TripGroupLocationModel::roleNames() const
{
    auto n = QAbstractListModel::roleNames();
    n.insert(LocationRole, "location");
    n.insert(LocationNameRole, "locationName");
    n.insert(LastUsedRole, "lastUsed");
    n.insert(UseCountRole, "useCount");
    n.insert(IsRemovableRole, "removable");
    return n;
}

void TripGroupLocationModel::populate()
{
    if (!m_tripGroupMgr) {
        return;
    }

    beginResetModel();
    m_locations.clear();

    const auto elems = m_tripGroupMgr->tripGroup(m_tripGroupId).elements();
    for (const auto &resId : elems) {
        const auto res = m_tripGroupMgr->reservationManager()->reservation(resId);
        if (LocationUtil::isLocationChange(res)) {
            Entry dep;
            dep.location = PublicTransport::locationFromPlace(LocationUtil::departureLocation(res), res);
            dep.lastUse = SortUtil::startDateTime(res);
            addEntry(std::move(dep));

            Entry arr;
            arr.location = PublicTransport::locationFromPlace(LocationUtil::arrivalLocation(res), res);
            arr.lastUse = SortUtil::endDateTime(res);
            addEntry(std::move(arr));
        } else {
            Entry e;
            e.location = PublicTransport::locationFromPlace(LocationUtil::location(res), res);
            e.lastUse = SortUtil::startDateTime(res);
            addEntry(std::move(e));
        }
    }
    endResetModel();

    Q_EMIT locationsChanged();
}

void TripGroupLocationModel::addEntry(Entry &&entry)
{
    if (entry.location.name().isEmpty() || !entry.location.hasCoordinate()) {
        return;
    }

    for (auto &loc : m_locations) {
        if (KPublicTransport::Location::isSame(loc.location, entry.location)) {
            loc.lastUse = std::max(loc.lastUse, entry.lastUse);
            loc.useCount += entry.useCount;
            return;
        }
    }
    m_locations.push_back(std::move(entry));
}

#include "moc_tripgrouplocationmodel.cpp"
