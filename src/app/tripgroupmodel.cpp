// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "tripgroupmodel.h"

#include "constants.h"
#include "logging.h"
#include "reservationmanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

#include <KItinerary/Flight>
#include <KItinerary/LocationUtil>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>

TripGroupModel::TripGroupModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_updateTimer.setTimerType(Qt::VeryCoarseTimer);
    m_updateTimer.setSingleShot(true);
    connect(&m_updateTimer, &QTimer::timeout, this, [this]() {
        if (m_tripGroups.empty()) {
            return;
        }
        Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0));
        scheduleUpdate();
    });

    connect(&m_currentBatchTimer, &QTimer::timeout, this, &TripGroupModel::currentBatchChanged);
    connect(&m_currentBatchTimer, &QTimer::timeout, this, &TripGroupModel::scheduleCurrentBatchTimer);
    m_currentBatchTimer.setTimerType(Qt::VeryCoarseTimer);
    m_currentBatchTimer.setSingleShot(true);
}

TripGroupModel::~TripGroupModel() = default;

TripGroupManager *TripGroupModel::tripGroupManager() const
{
    return m_tripGroupManager;
}

void TripGroupModel::setTripGroupManager(TripGroupManager *tripGroupManager)
{
    if (m_tripGroupManager == tripGroupManager) {
        return;
    }

    beginResetModel();
    m_tripGroupManager = tripGroupManager;
    m_tripGroups = m_tripGroupManager->tripGroups();
    std::sort(m_tripGroups.begin(), m_tripGroups.end(), [this](const auto &lhs, const auto &rhs) {
        return tripGroupLessThan(lhs, rhs);
    });

    connect(m_tripGroupManager, &TripGroupManager::tripGroupAdded, this, &TripGroupModel::tripGroupAdded);
    connect(m_tripGroupManager, &TripGroupManager::tripGroupChanged, this, &TripGroupModel::tripGroupChanged);
    connect(m_tripGroupManager, &TripGroupManager::tripGroupRemoved, this, &TripGroupModel::tripGroupRemoved);

    const auto currentBatchChangeHandler = [this]() {
        scheduleCurrentBatchTimer();
        Q_EMIT currentBatchChanged();
    };
    connect(m_tripGroupManager, &TripGroupManager::tripGroupAdded, this, currentBatchChangeHandler);
    connect(m_tripGroupManager, &TripGroupManager::tripGroupChanged, this, currentBatchChangeHandler);
    connect(m_tripGroupManager, &TripGroupManager::tripGroupRemoved, this, currentBatchChangeHandler);
    connect(m_tripGroupManager->reservationManager(), &ReservationManager::batchAdded, this, currentBatchChangeHandler);
    connect(m_tripGroupManager->reservationManager(), &ReservationManager::batchContentChanged, this, currentBatchChangeHandler);
    connect(m_tripGroupManager->reservationManager(), &ReservationManager::batchRemoved, this, currentBatchChangeHandler);

    endResetModel();
    scheduleUpdate();
    scheduleCurrentBatchTimer();

    Q_EMIT tripGroupManagerChanged();
    Q_EMIT currentBatchChanged();
}

QVariant TripGroupModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));

    const auto &tgId = m_tripGroups[index.row()];
    const auto tripGroup = m_tripGroupManager->tripGroup(tgId);

    switch (role) {
    case Qt::DisplayRole:
        return tripGroup.name();
    case BeginRole:
        return tripGroup.beginDateTime();
    case EndRole:
        return tripGroup.endDateTime();
    case PositionRole:
        // NOTE: when adjusting this adjust scheduleUpdate accordingly!
        if (today() > tripGroup.endDateTime().date()) {
            return Position::Past;
        }

        if (now() < tripGroup.beginDateTime().date().startOfDay()) {
            return Position::Future;
        }
        return Position::Current;
    case TripGroupRole:
        return QVariant::fromValue(tripGroup);
    case TripGroupIdRole:
        return tgId;
    default:
        return {};
    }
}

QHash<int, QByteArray> TripGroupModel::roleNames() const
{
    return {
        { Qt::DisplayRole, "name" },
        { BeginRole, "begin" },
        { EndRole, "end" },
        { PositionRole, "position" },
        { TripGroupRole, "tripGroup" },
        { TripGroupIdRole, "tripGroupId" },
    };
}

int TripGroupModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return int(m_tripGroups.size());
}

QStringList TripGroupModel::adjacentTripGroups(const QString &tripGroupId) const
{
    const auto tg = m_tripGroupManager->tripGroup(tripGroupId);
    auto tgIds = adjacentTripGroups(tg.beginDateTime(), tg.endDateTime());
    tgIds.removeAll(tripGroupId);
    return tgIds;
}

QStringList TripGroupModel::adjacentTripGroups(const QDateTime &from, const QDateTime &to) const
{
    auto it = std::lower_bound(m_tripGroups.begin(), m_tripGroups.end(), to, [this](const auto &lhs, const auto &rhs) {
        return tripGroupLessThan(lhs, rhs);
    });
    if (it != m_tripGroups.begin()) {
        --it;
    }

    QStringList res;
    if (it == m_tripGroups.end() && !m_tripGroups.empty()) {
        res.push_back(*std::prev(it));
    }

    for (;it != m_tripGroups.end(); ++it) {
        res.push_back(*it);
        if (from > m_tripGroupManager->tripGroup(*it).endDateTime()) {
            break;
        }
    }

    return res;
}

QStringList TripGroupModel::intersectingTripGroups(const QDateTime &from, const QDateTime &to) const
{
    auto it = std::lower_bound(m_tripGroups.begin(), m_tripGroups.end(), to, [this](const auto &lhs, const auto &rhs) {
        return tripGroupLessThan(lhs, rhs);
    });

    QStringList res;
    for (;it != m_tripGroups.end(); ++it) {
        const auto tg = m_tripGroupManager->tripGroup(*it);
        if (tg.beginDateTime() > to || tg.endDateTime() < from) {
            break;
        }
        res.push_back(*it);
    }

    return res;
}

// same as SortUtil::endDateTime but with a lower bound estimate
// when the end time isn't available
[[nodiscard]] static QDateTime estimatedEndTime(const QVariant &res)
{
    using namespace KItinerary;

    if (SortUtil::hasEndTime(res)) {
        return SortUtil::endDateTime(res);
    }
    if (JsonLd::isA<FlightReservation>(res)) {
        const auto flight = res.value<FlightReservation>().reservationFor().value<Flight>();
        const auto dist = LocationUtil::distance(flight.departureAirport().geo(), flight.arrivalAirport().geo());
        if (std::isnan(dist) || !flight.departureTime().isValid()) {
            return {};
        }
        auto dt = flight.departureTime();
        return dt.addSecs((qint64)(dist * 250.0 / 3.6)); // see flightutil.cpp in kitinerary
    }

    return {};
}

QString TripGroupModel::currentBatchId() const
{
    // find the closest trip group
    auto tgIds = intersectingTripGroups(now().addSecs(-Constants::CurrentBatchTrailingMargin.count()), now().addSecs(Constants::CurrentBatchLeadingMargin.count()));
    while (tgIds.size() > 1) { // tgIds is sorted by trip group start time
        const auto tg1 = m_tripGroupManager->tripGroup(tgIds.at(tgIds.size() - 1));
        const auto tg2 = m_tripGroupManager->tripGroup(tgIds.at(tgIds.size() - 2));
        if (!tg2.endDateTime().isValid()) {
            tgIds.remove(tgIds.size() - 2);
        }

        if (tg2.endDateTime().secsTo(now()) < now().secsTo(tg1.beginDateTime())) {
            tgIds.pop_back();
        } else {
            tgIds.remove(tgIds.size() - 2);
        }
    }
    if (tgIds.empty()) {
        return {};
    }

    // find the closest reservation
    using namespace KItinerary;
    const auto tg = m_tripGroupManager->tripGroup(tgIds.at(0));
    const auto elems = tg.elements(); // sorted chronologically

    QDateTime prevEndTime;
    QString prevResId;

    for (const auto &elem : elems) {
        const auto res = m_tripGroupManager->reservationManager()->reservation(elem);
        if (!LocationUtil::isLocationChange(res)) { // TODO also support event tickets!
            continue;
        }

        const auto startDt = SortUtil::startDateTime(res);
        const auto endDt = estimatedEndTime(res);
        const auto startDelta = now().secsTo(startDt);

        // currently ongoing
        if (endDt.isValid() && startDt < now() && now() < endDt) {
            return elem;
        }

        // next element is too far out
        if (startDelta > Constants::CurrentBatchLeadingMargin.count()) {
            return prevResId;
        }

        // next element is in range, and there is no viable previous element
        if (!prevEndTime.isValid() && startDelta > 0 && startDelta < Constants::CurrentBatchLeadingMargin.count()) {
            return elem;
        }
        // both next and previous are viable, pick the closest one
        if (prevEndTime.isValid() && prevEndTime < now() && now() < startDt) {
            return prevEndTime.secsTo(now()) < startDelta ? prevResId : elem;
        }

        // ended not too long ago
        const auto endDelta = endDt.secsTo(now());
        if (endDt.isValid() && endDelta > 0 && endDelta < Constants::CurrentBatchTrailingMargin.count()) {
            prevResId = elem;
            prevEndTime = endDt;
        }
    }

    return {};
}

void TripGroupModel::tripGroupAdded(const QString &tgId)
{
    const auto it = std::lower_bound(m_tripGroups.begin(), m_tripGroups.end(), tgId, [this](const auto &lhs, const auto &rhs) {
        return tripGroupLessThan(lhs, rhs);
    });
    const auto row = (int)std::distance(m_tripGroups.begin(), it);
    beginInsertRows({}, row, row);
    m_tripGroups.insert(it, tgId);
    endInsertRows();
    scheduleUpdate();
}

void TripGroupModel::tripGroupChanged(const QString &tgId)
{
    const auto it = std::find(m_tripGroups.begin(), m_tripGroups.end(), tgId);
    if (it == m_tripGroups.end()) {
        qCCritical(Log) << "got change for an unknown trip group";
        tripGroupAdded(tgId);
        return;
    }

    // check if sort order changed
    bool order = true;
    if (it != m_tripGroups.begin()) {
        order &= tripGroupLessThan(*std::prev(it), tgId);
    }
    if (std::next(it) != m_tripGroups.end()) {
        order &= tripGroupLessThan(tgId, *std::next(it));
    }

    if (order) {
        const auto row = (int)std::distance(m_tripGroups.begin(), it);
        const auto idx = index(row, 0);
        scheduleUpdate();
        Q_EMIT dataChanged(idx, idx);
    } else {
        tripGroupRemoved(tgId);
        tripGroupAdded(tgId);
    }
}

void TripGroupModel::tripGroupRemoved(const QString &tgId)
{
    const auto it = std::find(m_tripGroups.begin(), m_tripGroups.end(), tgId);
    if (it == m_tripGroups.end()) {
        return;
    }
    const auto row = (int)std::distance(m_tripGroups.begin(), it);
    beginRemoveRows({}, row, row);
    m_tripGroups.erase(it);
    endRemoveRows();
    scheduleUpdate();
}

bool TripGroupModel::tripGroupLessThan(const QString &lhs, const QString &rhs) const
{
    return m_tripGroupManager->tripGroup(lhs).beginDateTime() > m_tripGroupManager->tripGroup(rhs).beginDateTime();
}

bool TripGroupModel::tripGroupLessThan(const QString &lhs, const QDateTime &rhs) const
{
    return m_tripGroupManager->tripGroup(lhs).beginDateTime() > rhs;
}

void TripGroupModel::setCurrentDateTime(const QDateTime &dt)
{
    m_unitTestTime = dt;
}

void TripGroupModel::scheduleUpdate()
{
    const auto currentDt = now();

    // find first current or future element
    auto it = std::lower_bound(m_tripGroups.begin(), m_tripGroups.end(), currentDt, [this](const auto &lhs, const auto &rhs) {
        return tripGroupLessThan(lhs, rhs);
    });
    while (it != m_tripGroups.begin() && (it == m_tripGroups.end() || m_tripGroupManager->tripGroup(*it).endDateTime() > currentDt)) {
        --it;
    }

    QDateTime dt;
    for (; it != m_tripGroups.end(); ++it) {
        const auto tg = m_tripGroupManager->tripGroup(*it);
        if (dt.isValid() && tg.beginDateTime().date().startOfDay() > dt) {
            break;
        }
        if (const auto beginDt = tg.beginDateTime().date().startOfDay(); beginDt > currentDt) {
            dt = dt.isValid() ? std::min(dt, beginDt) : beginDt;
        }
        if (const auto endDt = tg.endDateTime().date().endOfDay(); endDt > currentDt) {
            dt = dt.isValid() ? std::min(dt, endDt) : endDt;
        }
    }

    if (!dt.isValid()) {
        return;
    }

    const auto timeDelta = std::chrono::seconds(std::max(60ll, currentDt.secsTo(dt)));
    if (timeDelta.count() * 1000 > std::numeric_limits<int>::max()) {
        qCDebug(Log) << "not scheduling trip group model update, would overflow QTimer" << dt;
        m_updateTimer.start(std::numeric_limits<int>::max());
    } else {
        qCDebug(Log) << "scheduling next trip group update for" << dt;
        m_updateTimer.start(timeDelta);
    }
}

void TripGroupModel::scheduleCurrentBatchTimer()
{
    // find relevant trip groups
    auto tgIds = intersectingTripGroups(now().addSecs(-Constants::CurrentBatchTrailingMargin.count()), now().addSecs(Constants::CurrentBatchLeadingMargin.count()));
    while (tgIds.size() > 1) { // tgIds is sorted by trip group start time
        const auto tg1 = m_tripGroupManager->tripGroup(tgIds.at(tgIds.size() - 1));
        const auto tg2 = m_tripGroupManager->tripGroup(tgIds.at(tgIds.size() - 2));
        if (!tg2.endDateTime().isValid()) {
            tgIds.remove(tgIds.size() - 2);
        }

        if (tg2.endDateTime().secsTo(now()) < now().secsTo(tg1.beginDateTime())) {
            tgIds.pop_back();
        } else {
            tgIds.remove(tgIds.size() - 2);
        }
    }

    // we need the smallest valid time > now() of any of the following:
    // - end time of the current element + margin
    // - start time - margin of the next res
    // - end time of current + start time of next - endtime of current / 2
    // - end time of the next element (in case we are in the leading margin already)

    QDateTime triggerTime;
    const auto updateTriggerTime = [&triggerTime, this](const QDateTime &dt) {
        if (!dt.isValid() || dt <= now()) {
            return;
        }
        if (!triggerTime.isValid()) {
            triggerTime = dt;
        } else {
            triggerTime = std::min(triggerTime, dt);
        }
    };

    // rather brute-force, but good enough as we are only looking at a ~48h time window here anyway
    using namespace KItinerary;
    for (const auto &tgId : tgIds) {
        const auto tg = m_tripGroupManager->tripGroup(tgId);
        const auto elems = tg.elements();
        QDateTime endDt;
        for (const auto &elem : elems) {
            const auto res = m_tripGroupManager->reservationManager()->reservation(elem);
            const auto startDt = SortUtil::startDateTime(res);
            updateTriggerTime(startDt.addSecs(-Constants::CurrentBatchLeadingMargin.count()));
            if (endDt.isValid()) {
                updateTriggerTime(endDt.addSecs(endDt.secsTo(startDt) / 2));
            }
            endDt = estimatedEndTime(res);
            updateTriggerTime(endDt);
            updateTriggerTime(endDt.addSecs(Constants::CurrentBatchTrailingMargin.count()));
        }
    }

    if (!triggerTime.isValid()) {
        return;
    }
    // QTimer only has 31bit for its msec interval
    if (now().secsTo(triggerTime) * 1000 > std::numeric_limits<int>::max()) {
        qCDebug(Log) << "not scheduling trip group model update, would overflow QTimer" << triggerTime;
        m_currentBatchTimer.start(std::numeric_limits<int>::max());
    } else {
        m_currentBatchTimer.setInterval(std::chrono::seconds(std::max<qint64>(60, now().secsTo(triggerTime))));
        m_currentBatchTimer.start();
    }
}

QDateTime TripGroupModel::now() const
{
    if (Q_UNLIKELY(m_unitTestTime.isValid())) {
        return m_unitTestTime;
    }
    return QDateTime::currentDateTime();
}

QDate TripGroupModel::today() const
{
    if (Q_UNLIKELY(m_unitTestTime.isValid())) {
        return m_unitTestTime.date();
    }
    return QDate::currentDate();
}

#include "moc_tripgroupmodel.cpp"
