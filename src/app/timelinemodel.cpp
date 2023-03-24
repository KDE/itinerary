/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "timelinemodel.h"
#include "constants.h"
#include "locationhelper.h"
#include "locationinformation.h"
#include "pkpassmanager.h"
#include "reservationmanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"
#include "transfermanager.h"
#include "weatherinformation.h"

#include <weatherforecast.h>
#include <weatherforecastmanager.h>

#include <KItinerary/BusTrip>
#include <KItinerary/CountryDb>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/LocationUtil>
#include <KItinerary/MergeUtil>
#include <KItinerary/Organization>
#include <KItinerary/Reservation>
#include <KItinerary/SortUtil>
#include <KItinerary/TrainTrip>
#include <KItinerary/Visit>

#include <KPkPass/Pass>

#include <KLocalizedString>

#include <QDebug>
#include <QLocale>

#include <cassert>

using namespace KItinerary;

static bool needsSplitting(const QVariant &res)
{
    // multi-day event?
    if (JsonLd::isA<EventReservation>(res)) {
        const auto ev = res.value<EventReservation>().reservationFor().value<Event>();
        return ev.startDate().date() != ev.endDate().date() && ev.endDate().isValid();
    }

    return JsonLd::isA<LodgingReservation>(res)
        || JsonLd::isA<RentalCarReservation>(res);
}

static QTimeZone timeZone(const QDateTime &dt)
{
    return dt.timeSpec() == Qt::TimeZone ? dt.timeZone() : QTimeZone();
}

TimelineModel::TimelineModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&m_dayUpdateTimer, &QTimer::timeout, this, &TimelineModel::dayChanged);
    m_dayUpdateTimer.setTimerType(Qt::VeryCoarseTimer);
    m_dayUpdateTimer.setSingleShot(true);
    m_dayUpdateTimer.setInterval((QTime::currentTime().secsTo({23, 59, 59}) + 1) * 1000);
    m_dayUpdateTimer.start();

    // make sure we properly update the empty today marker
    connect(this, &TimelineModel::todayRowChanged, this, [this]() {
        const auto idx = index(todayRow(), 0);
        if (m_todayEmpty == idx.data(TimelineModel::TodayEmptyRole).toBool()) {
            return;
        }
        m_todayEmpty = !m_todayEmpty;
        Q_EMIT dataChanged(idx, idx);
    });

    connect(&m_currentBatchTimer, &QTimer::timeout, this, &TimelineModel::currentBatchChanged);
    connect(&m_currentBatchTimer, &QTimer::timeout, this, &TimelineModel::updateTodayMarker);
    connect(&m_currentBatchTimer, &QTimer::timeout, this, &TimelineModel::scheduleCurrentBatchTimer);
    m_currentBatchTimer.setTimerType(Qt::VeryCoarseTimer);
    m_currentBatchTimer.setSingleShot(true);
}

TimelineModel::~TimelineModel() = default;

void TimelineModel::setReservationManager(ReservationManager* mgr)
{
    // for auto tests only
    if (Q_UNLIKELY(!mgr)) {
        beginResetModel();
        disconnect(m_resMgr, &ReservationManager::batchAdded, this, &TimelineModel::batchAdded);
        disconnect(m_resMgr, &ReservationManager::batchChanged, this, &TimelineModel::batchChanged);
        disconnect(m_resMgr, &ReservationManager::batchContentChanged, this, &TimelineModel::batchChanged);
        disconnect(m_resMgr, &ReservationManager::batchRenamed, this, &TimelineModel::batchRenamed);
        disconnect(m_resMgr, &ReservationManager::batchRemoved, this, &TimelineModel::batchRemoved);
        m_resMgr = mgr;
        m_elements.clear();
        endResetModel();
        return;
    }

    beginResetModel();
    m_resMgr = mgr;
    for (const auto &resId : mgr->batches()) {
        const auto res = m_resMgr->reservation(resId);
        auto elem = TimelineElement(this, resId, res, TimelineElement::SelfContained);
        if (!elem.isReservation()) { // a type we can't handle
            continue;
        }
        if (needsSplitting(res)) {
            m_elements.push_back(TimelineElement{this, resId, res, TimelineElement::RangeBegin});
            m_elements.push_back(TimelineElement{this, resId, res, TimelineElement::RangeEnd});
        } else {
            m_elements.push_back(std::move(elem));
        }
    }
    m_elements.push_back(TimelineElement{this, TimelineElement::TodayMarker,  QDateTime(today(), QTime(0, 0))});
    std::sort(m_elements.begin(), m_elements.end());

    connect(mgr, &ReservationManager::batchAdded, this, &TimelineModel::batchAdded);
    connect(mgr, &ReservationManager::batchChanged, this, &TimelineModel::batchChanged);
    connect(mgr, &ReservationManager::batchContentChanged, this, &TimelineModel::batchChanged);
    connect(mgr, &ReservationManager::batchRenamed, this, &TimelineModel::batchRenamed);
    connect(mgr, &ReservationManager::batchRemoved, this, &TimelineModel::batchRemoved);
    endResetModel();

    updateTodayMarker();
    updateInformationElements();
    Q_EMIT todayRowChanged();

    scheduleCurrentBatchTimer();
    Q_EMIT currentBatchChanged();
}

void TimelineModel::setWeatherForecastManager(WeatherForecastManager* mgr)
{
    m_weatherMgr = mgr;
    updateWeatherElements();
    connect(m_weatherMgr, &WeatherForecastManager::forecastUpdated, this, &TimelineModel::updateWeatherElements);
}

TripGroupManager* TimelineModel::tripGroupManager() const
{
    return m_tripGroupManager;
}

void TimelineModel::setTripGroupManager(TripGroupManager *mgr)
{
    m_tripGroupManager = mgr;
    connect(mgr, &TripGroupManager::tripGroupAdded, this, &TimelineModel::tripGroupAdded);
    connect(mgr, &TripGroupManager::tripGroupChanged, this, &TimelineModel::tripGroupChanged);
    connect(mgr, &TripGroupManager::tripGroupRemoved, this, &TimelineModel::tripGroupRemoved);
    for (const auto &group : mgr->tripGroups()) {
        tripGroupAdded(group);
    }
}

void TimelineModel::setHomeCountryIsoCode(const QString &isoCode)
{
    m_homeCountry = isoCode;
    updateInformationElements();
}

void TimelineModel::setTransferManager(TransferManager *mgr)
{
    m_transferManager = mgr;
    connect(mgr, &TransferManager::transferAdded, this, &TimelineModel::transferChanged);
    connect(mgr, &TransferManager::transferChanged, this, &TimelineModel::transferChanged);
    connect(mgr, &TransferManager::transferRemoved, this, &TimelineModel::transferRemoved);

    // load existing transfers into the model
    for (const auto &batchId : m_resMgr->batches()) {
        updateTransfersForBatch(batchId);
    }
}

int TimelineModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_resMgr) {
        return 0;
    }
    return m_elements.size();
}

QVariant TimelineModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_resMgr) {
        return {};
    }

    const auto &elem = m_elements.at(index.row());
    switch (role) {
        case SectionHeaderRole:
            // see TimelineSectionDelegateController
            return elem.dt.date().toString(Qt::ISODate);
        case BatchIdRole:
            return elem.isReservation() ? elem.batchId() : QString();
        case ElementTypeRole:
            return elem.elementType;
        case TodayEmptyRole:
            if (elem.elementType == TimelineElement::TodayMarker) {
                return isDateEmpty(m_elements.at(index.row()).dt.date());
            }
            return {};
        case IsTodayRole:
            return elem.dt.date() == today();
        case ElementRangeRole:
            return elem.rangeType;
        case LocationInformationRole:
            if (elem.elementType == TimelineElement::LocationInfo)
                return elem.content();
            break;
        case WeatherForecastRole:
            if (elem.elementType == TimelineElement::WeatherForecast)
                return elem.content();
            break;
        case ReservationsRole:
        {
            if (!elem.isReservation()) {
                return {};
            }
            const auto resIds = m_resMgr->reservationsForBatch(elem.batchId());
            QVector<QVariant> v;
            v.reserve(resIds.size());
            for (const auto &resId : resIds) {
                v.push_back(m_resMgr->reservation(resId));
            }
            std::sort(v.begin(), v.end(), SortUtil::isBefore);
            return QVariant::fromValue(v);
        }
        case TripGroupIdRole:
            if (elem.elementType == TimelineElement::TripGroup) {
                return elem.content();
            }
            break;
        case TripGroupRole:
            if (elem.elementType == TimelineElement::TripGroup)
                return QVariant::fromValue(m_tripGroupManager->tripGroup(elem.content().toString()));
            break;
        case TransferRole:
            if (elem.elementType == TimelineElement::Transfer) {
                return elem.content();
            }
            break;
        case StartDateTimeRole:
            return elem.dt;
        case EndDateTimeRole:
            return elem.endDateTime();
        case IsTimeboxedRole:
            return elem.isTimeBoxed();
        case IsCanceledRole:
            return elem.isCanceled();
    }
    return {};
}

QHash<int, QByteArray> TimelineModel::roleNames() const
{
    auto names = QAbstractListModel::roleNames();
    names.insert(SectionHeaderRole, "sectionHeader");
    names.insert(BatchIdRole, "batchId");
    names.insert(ElementTypeRole, "type");
    names.insert(TodayEmptyRole, "isTodayEmpty");
    names.insert(IsTodayRole, "isToday");
    names.insert(ElementRangeRole, "rangeType");
    names.insert(LocationInformationRole, "locationInformation");
    names.insert(WeatherForecastRole, "weatherInformation");
    names.insert(ReservationsRole, "reservations");
    names.insert(TripGroupIdRole, "tripGroupId");
    names.insert(TripGroupRole, "tripGroup");
    names.insert(TransferRole, "transfer");
    return names;
}

int TimelineModel::todayRow() const
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [](const auto &e) { return e.elementType == TimelineElement::TodayMarker; });
    return std::distance(m_elements.begin(), it);
}

void TimelineModel::batchAdded(const QString &resId)
{
    const auto res = m_resMgr->reservation(resId);
    if (needsSplitting(res)) {
        insertElement(TimelineElement{this, resId, res, TimelineElement::RangeBegin});
        insertElement(TimelineElement{this, resId, res, TimelineElement::RangeEnd});
    } else {
        insertElement(TimelineElement{this, resId, res, TimelineElement::SelfContained});
    }

    updateInformationElements();
    updateTransfersForBatch(resId);
    Q_EMIT todayRowChanged();

    scheduleCurrentBatchTimer();
    Q_EMIT currentBatchChanged();
}

void TimelineModel::insertElement(TimelineElement &&elem)
{
    if (elem.elementType == TimelineElement::Undefined) {
        return;
    }

    auto it = std::lower_bound(m_elements.begin(), m_elements.end(), elem);
    const auto row = std::distance(m_elements.begin(), it);

    beginInsertRows({}, row, row);
    m_elements.insert(it, std::move(elem));
    endInsertRows();
}

std::vector<TimelineElement>::iterator TimelineModel::insertOrUpdate(std::vector<TimelineElement>::iterator it, TimelineElement &&elem)
{
    assert(elem.elementType != TimelineElement::Undefined);

    while (it != m_elements.end() && (*it) < elem) {
        ++it;
    }

    if (it != m_elements.end() && (*it) == elem) {
        const auto row = std::distance(m_elements.begin(), it);
        (*it) = std::move(elem);
        Q_EMIT dataChanged(index(row, 0), index(row, 0));
    } else {
        const auto row = std::distance(m_elements.begin(), it);
        beginInsertRows({}, row, row);
        it = m_elements.insert(it, std::move(elem));
        endInsertRows();
    }
    return it;
}

void TimelineModel::batchChanged(const QString &resId)
{
    const auto res = m_resMgr->reservation(resId);
    if (needsSplitting(res)) {
        updateElement(resId, res, TimelineElement::RangeBegin);
        updateElement(resId, res, TimelineElement::RangeEnd);
    } else {
        updateElement(resId, res, TimelineElement::SelfContained);
    }

    updateInformationElements();

    scheduleCurrentBatchTimer();
    Q_EMIT currentBatchChanged();
}

void TimelineModel::batchRenamed(const QString& oldBatchId, const QString& newBatchId)
{
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
        if (!(*it).isReservation() || (*it).batchId() != oldBatchId) {
            continue;
        }

        (*it).setContent(newBatchId);
        const auto idx = index(std::distance(m_elements.begin(), it), 0);
        Q_EMIT dataChanged(idx, idx);

        if ((*it).rangeType == TimelineElement::SelfContained || (*it).rangeType == TimelineElement::RangeEnd) {
            break;
        }
    }
}

void TimelineModel::updateElement(const QString &resId, const QVariant &res, TimelineElement::RangeType rangeType)
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [resId, rangeType](const auto &e) {
        return e.isReservation() && e.batchId() == resId && e.rangeType == rangeType;
    });
    if (it == m_elements.end()) {
        return;
    }
    const auto row = std::distance(m_elements.begin(), it);
    const auto newDt = TimelineElement::relevantDateTime(res, rangeType);

    if ((*it).dt != newDt) {
        // element moved
        beginRemoveRows({}, row, row);
        m_elements.erase(it);
        endRemoveRows();
        insertElement(TimelineElement{this, resId, res, rangeType});
    } else {
        Q_EMIT dataChanged(index(row, 0), index(row, 0));
    }
}

void TimelineModel::batchRemoved(const QString &resId)
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [resId](const auto &e) {
        return e.isReservation() && e.batchId() == resId;
    });
    if (it == m_elements.end()) {
        return;
    }
    const auto isSplit = (*it).rangeType == TimelineElement::RangeBegin;
    const auto row = std::distance(m_elements.begin(), it);

    beginRemoveRows({}, row, row);
    m_elements.erase(it);
    endRemoveRows();
    Q_EMIT todayRowChanged();

    if (isSplit) {
        batchRemoved(resId);
    }

    updateInformationElements();

    scheduleCurrentBatchTimer();
    Q_EMIT currentBatchChanged();
}

void TimelineModel::dayChanged()
{
    updateTodayMarker();
    updateWeatherElements();

    m_dayUpdateTimer.setInterval((QTime::currentTime().secsTo({23, 59, 59}) + 1) * 1000);
    m_dayUpdateTimer.start();

    scheduleCurrentBatchTimer();
    Q_EMIT currentBatchChanged();
}

void TimelineModel::updateTodayMarker()
{
    auto dt = now();
    auto it = std::lower_bound(m_elements.begin(), m_elements.end(), dt);

    if (it != m_elements.begin()) {
        const auto prevIt = std::prev(it);
        // check if the previous element is the old today marker, if so nothing to do
        if ((*prevIt).elementType == TimelineElement::TodayMarker) {
            (*prevIt).dt = dt;
            return;
        }
        // check if the previous element is still ongoing, in that case we want to be before that
        if ((*prevIt).dt.date() == today() && ((*prevIt).isTimeBoxed() && (*prevIt).endDateTime() > now())) {
            it = prevIt;
            dt = (*prevIt).dt;
        }
    }

    const auto newRow = std::distance(m_elements.begin(), it);
    const auto oldRow = todayRow();
    if (oldRow >= newRow) {
        return;
    }

    beginInsertRows({}, newRow, newRow);
    m_elements.insert(it, TimelineElement{this, TimelineElement::TodayMarker, dt});
    endInsertRows();

    beginRemoveRows({}, oldRow, oldRow);
    m_elements.erase(m_elements.begin() + oldRow);
    endRemoveRows();
    Q_EMIT todayRowChanged();
}

void TimelineModel::updateInformationElements()
{
    // the location information is shown after location changes or before stationary elements
    // when transitioning into a location that:
    // - differs in one or more properties from the home country, and the difference
    // was introduced by this transtion
    // - differs in timezone from the previous location, and that timezone has a different
    // offset at the time of transition

    LocationInformation homeCountry;
    homeCountry.setIsoCode(m_homeCountry);

    auto previousCountry = homeCountry;
    for (auto it = m_elements.begin(); it != m_elements.end();) {
        if ((*it).elementType == TimelineElement::LocationInfo) { // this is one we didn't generate, otherwise it would be beyond that
            const auto row = std::distance(m_elements.begin(), it);
            beginRemoveRows({}, row, row);
            it = m_elements.erase(it);
            endRemoveRows();
            continue;
        }

        if ((*it).isCanceled() || (*it).isInformational()) {
            ++it;
            continue;
        }

        auto newCountry = homeCountry;
        newCountry.setIsoCode(LocationUtil::address((*it).destination()).addressCountry());
        newCountry.setTimeZone(previousCountry.timeZone(), (*it).dt);
        newCountry.setTimeZone(timeZone((*it).endDateTime()), (*it).dt);
        if (newCountry == previousCountry) {
            ++it;
            continue;
        }
        if (!(newCountry == homeCountry) || newCountry.hasRelevantTimeZoneChange(previousCountry)) {
            // for location changes, we want this after the corresponding element
            const auto dt = (*it).isLocationChange() ? (*it).endDateTime() : (*it).dt;
            it = insertOrUpdate(it, TimelineElement{this, TimelineElement::LocationInfo, dt, QVariant::fromValue(newCountry)});
        }

        ++it;
        previousCountry = newCountry;
    }

    updateWeatherElements();
}

void TimelineModel::updateWeatherElements()
{
    if (!m_weatherMgr || !m_weatherMgr->allowNetworkAccess() || m_elements.empty()) {
        return;
    }

    qDebug() << "recomputing weather elements";
    GeoCoordinates geo;
    QString label;

    auto date = now();
    // round to next full hour
    date.setTime(QTime(date.time().hour(), 0));
    date = date.addSecs(60 * 60);
    const auto maxForecastTime = m_weatherMgr->maximumForecastTime(date.date());

    // look through the past, clean up weather elements there and figure out where we are
    auto it = m_elements.begin();
    for (; it != m_elements.end() && (*it).dt < now();) {
        if ((*it).elementType == TimelineElement::WeatherForecast) {
            const auto row = std::distance(m_elements.begin(), it);
            beginRemoveRows({}, row, row);
            it = m_elements.erase(it);
            endRemoveRows();
            continue;
        }

        if ((*it).isCanceled()) {
            ++it;
            continue;
        }
        const auto newGeo = LocationUtil::geo((*it).destination());
        if ((*it).isLocationChange() || newGeo.isValid()) {
            geo = newGeo;
            label = WeatherInformation::labelForPlace((*it).destination());

            // if we are in an ongoing location change, start afterwards
            const auto endDt = (*it).endDateTime();
            if ((*it).isLocationChange() && endDt.isValid() && date < endDt) {
                date = endDt;
            }
        }

        ++it;
    }

    while(it != m_elements.end() && date < maxForecastTime) {

        if ((*it).dt < date || (*it).elementType == TimelineElement::TodayMarker) {
            // clean up outdated weather elements (happens when merging previously split ranges)
            if ((*it).elementType == TimelineElement::WeatherForecast) {
                const auto row = std::distance(m_elements.begin(), it);
                beginRemoveRows({}, row, row);
                it = m_elements.erase(it);
                endRemoveRows();
                if (it == m_elements.end()) {
                    break;
                }
                continue;
            }

            // track where we are
            if ((*it).isCanceled()) {
                ++it;
                continue;
            }
            const auto newGeo = LocationUtil::geo((*it).destination());
            if ((*it).isLocationChange() || newGeo.isValid()) {
                geo = newGeo;
                label = WeatherInformation::labelForPlace((*it).destination());
            }

            ++it;
            continue;
        }

        // determine the length of the forecast range (at most until the end of the day)
        auto endTime = date;
        endTime.setTime(QTime(23, 59, 59));
        auto nextStartTime = endTime;
        GeoCoordinates newGeo = geo;
        QString newLabel = label;
        for (auto it2 = it; it2 != m_elements.end(); ++it2) {
            if ((*it2).dt >= endTime) {
                break;
            }
            if ((*it2).isLocationChange()) {
                // exclude the actual travel time from forecast ranges
                endTime = std::min(endTime, (*it2).dt);
                nextStartTime = std::max(endTime, (*it2).endDateTime());
                newGeo = LocationUtil::geo((*it2).destination());
                newLabel = WeatherInformation::labelForPlace((*it2).destination());
                break;
            }
        }

        ::WeatherForecast fc;
        if (geo.isValid()) {
            m_weatherMgr->monitorLocation(geo.latitude(), geo.longitude());
            fc = m_weatherMgr->forecast(geo.latitude(), geo.longitude(), date, endTime);
        }

        // updated or new data
        if (fc.isValid()) {
            it = insertOrUpdate(it, TimelineElement{this, TimelineElement::WeatherForecast, date, QVariant::fromValue(WeatherInformation{fc, label})});
        }
        // we have no forecast data, but a matching weather element: remove
        else if ((*it).elementType == TimelineElement::WeatherForecast && (*it).dt == date) {
            const auto row = std::distance(m_elements.begin(), it);
            beginRemoveRows({}, row, row);
            it = m_elements.erase(it);
            endRemoveRows();
        }

        geo = newGeo;
        label = newLabel;
        date = nextStartTime.addSecs(1);
        ++it;
    }

    // append weather elements beyond the end of the list if necessary
    while (date < maxForecastTime && geo.isValid()) {
        auto endTime = date;
        endTime.setTime(QTime(23, 59, 59));

        m_weatherMgr->monitorLocation(geo.latitude(), geo.longitude());
        const auto fc = m_weatherMgr->forecast(geo.latitude(), geo.longitude(), date, endTime);
        if (fc.isValid()) {
            const auto row = std::distance(m_elements.begin(), it);
            beginInsertRows({}, row, row);
            it = m_elements.insert(it, TimelineElement{this, TimelineElement::WeatherForecast, date, QVariant::fromValue(WeatherInformation{fc, label})});
            ++it;
            endInsertRows();
        }
        date = endTime.addSecs(1);
    }

    qDebug() << "weather recomputation done";
}

void TimelineModel::updateTransfersForBatch(const QString& batchId)
{
    if (!m_transferManager) {
        return;
    }

    auto transfer = m_transferManager->transfer(batchId, Transfer::Before);
    if (transfer.state() != Transfer::UndefinedState) {
        transferChanged(transfer);
    }
    transfer = m_transferManager->transfer(batchId, Transfer::After);
    if (transfer.state() != Transfer::UndefinedState) {
        transferChanged(transfer);
    }
}

QDateTime TimelineModel::now() const
{
    if (Q_UNLIKELY(m_unitTestTime.isValid())) {
        return m_unitTestTime;
    }
    return QDateTime::currentDateTime();
}

QDate TimelineModel::today() const
{
    if (Q_UNLIKELY(m_unitTestTime.isValid())) {
        return m_unitTestTime.date();
    }
    return QDate::currentDate();
}

void TimelineModel::setCurrentDateTime(const QDateTime &dt)
{
    const auto dayDiffers = today() != dt.date();
    m_unitTestTime = dt;
    if (dayDiffers && !m_elements.empty()) {
        dayChanged();
    }

    scheduleCurrentBatchTimer();
}

void TimelineModel::tripGroupAdded(const QString& groupId)
{
    const auto g = m_tripGroupManager->tripGroup(groupId);

    TimelineElement beginElem{this, TimelineElement::TripGroup, g.beginDateTime(), groupId};
    beginElem.rangeType = TimelineElement::RangeBegin;
    insertElement(std::move(beginElem));

    TimelineElement endElem{this, TimelineElement::TripGroup, g.endDateTime(), groupId};
    endElem.rangeType = TimelineElement::RangeEnd;
    insertElement(std::move(endElem));
}

void TimelineModel::tripGroupChanged(const QString& groupId)
{
    // ### this can be done better probably
    tripGroupRemoved(groupId);
    tripGroupAdded(groupId);
}

void TimelineModel::tripGroupRemoved(const QString& groupId)
{
    for (auto it = m_elements.begin(); it != m_elements.end();) {
        if ((*it).elementType != TimelineElement::TripGroup || (*it).content().toString() != groupId) {
            ++it;
            continue;
        }

        const auto row = std::distance(m_elements.begin(), it);
        beginRemoveRows({}, row, row);
        it = m_elements.erase(it);
        endRemoveRows();
    }
}

void TimelineModel::transferChanged(const Transfer& transfer)
{
    if (transfer.state() == Transfer::UndefinedState) {
        return;
    }
    if (transfer.state() == Transfer::Discarded) {
        transferRemoved(transfer.reservationId(), transfer.alignment());
        return;
    }

    auto it = std::find_if(m_elements.begin(), m_elements.end(), [transfer](const auto &e) {
        return e.isReservation() && e.batchId() == transfer.reservationId();
    });
    if (it == m_elements.end()) {
        return;
    }

    if (transfer.alignment() == Transfer::Before) {
        if (it != m_elements.begin()) {
            --it;
        }
    }
    insertOrUpdate(it, TimelineElement(this, transfer));

    Q_EMIT todayRowChanged();
}

void TimelineModel::transferRemoved(const QString &resId, Transfer::Alignment alignment)
{
    auto it = std::find_if(m_elements.begin(), m_elements.end(), [resId, alignment](const auto &e) {
        return e.elementType == TimelineElement::Transfer && e.batchId() == resId
            && e.content().template value<Transfer>().alignment() == alignment;
    });
    if (it == m_elements.end()) {
        return;
    }

    const auto row = std::distance(m_elements.begin(), it);
    beginRemoveRows({}, row, row);
    m_elements.erase(it);
    endRemoveRows();

    Q_EMIT todayRowChanged();
}

static bool isSelectableElement(const TimelineElement &elem)
{
    return elem.isReservation() && elem.rangeType == TimelineElement::SelfContained;
}

QString TimelineModel::currentBatchId() const
{
    if (m_elements.empty()) {
        return {};
    }

    // find the next reservation
    auto it = std::lower_bound(m_elements.begin(), m_elements.end(), now());
    for (; it != m_elements.end() && !isSelectableElement(*it); ++it) {}

    QString nextResId;
    QDateTime nextStartTime;
    if (it != m_elements.end() && now().secsTo((*it).dt) < Constants::CurrentBatchLeadingMargin.count()) {
        nextResId = (*it).batchId();
        nextStartTime = (*it).dt;
    }

    // find the previous or current reservation
    if (it != m_elements.begin()) {
        --it;
        for (; it != m_elements.begin() && (it == m_elements.end() || !isSelectableElement(*it)); --it) {}
    }

    const auto resId = (*it).batchId();
    const auto res = m_resMgr->reservation(resId);
    auto endTime = SortUtil::endDateTime(res);
    if (endTime.secsTo(now()) > Constants::CurrentBatchTrailingMargin.count()) {
        endTime = {};
    }

    // only one side found
    if (!endTime.isValid()) {
        return nextResId;
    }
    if (!nextStartTime.isValid()) {
        return resId;
    }

    // (*it) is still active
    if (endTime >= now()) {
        return resId;
    }

    // take the one that is closer
    return endTime.secsTo(now()) < now().secsTo(nextStartTime) ? resId : nextResId;
}

void TimelineModel::scheduleCurrentBatchTimer()
{
    if (m_elements.empty()) {
        return;
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

    // find the next reservation
    auto it = std::lower_bound(m_elements.begin(), m_elements.end(), now());
    for (; it != m_elements.end() && !isSelectableElement(*it); ++it) {}

    QDateTime nextStartTime;
    if (it != m_elements.end()) {
        nextStartTime = (*it).dt;
        updateTriggerTime(nextStartTime.addSecs(-Constants::CurrentBatchLeadingMargin.count()));
        updateTriggerTime(SortUtil::endDateTime(m_resMgr->reservation((*it).batchId())));
    }

    // find the previous or current reservation
    if (it != m_elements.begin()) {
        --it;
        for (; it != m_elements.begin() && (it == m_elements.end() || !isSelectableElement(*it)); --it) {}
    }

    const auto res = m_resMgr->reservation((*it).batchId());
    auto endTime = SortUtil::endDateTime(res);
    updateTriggerTime(endTime.addSecs(Constants::CurrentBatchTrailingMargin.count()));

    if (nextStartTime.isValid() && endTime.isValid()) {
        updateTriggerTime(endTime.addSecs(endTime.secsTo(nextStartTime) / 2));
    }

    // QTimer only has 31bit for its msec interval, so don't schedule beyond a day
    // for longer distances we re-run this in the midnight timer above
    if (triggerTime.isValid() && triggerTime.date() == today()) {
        m_currentBatchTimer.setInterval(std::chrono::seconds(std::max<qint64>(60, now().secsTo(triggerTime))));
        m_currentBatchTimer.start();
    }
}

QVariant TimelineModel::locationAtTime(const QDateTime& dt) const
{
    auto it = std::lower_bound(m_elements.begin(), m_elements.end(), dt);
    if (it == m_elements.begin()) {
        return {};
    }

    for (--it ;; --it) {
        // this is a still ongoing non-location change
        if (it != m_elements.end() && !(*it).isLocationChange() && (*it).endDateTime().isValid() && (*it).endDateTime() > dt) {
            if ((*it).isReservation()) {
                auto loc = LocationUtil::location(m_resMgr->reservation((*it).batchId()));
                if (LocationUtil::geo(loc).isValid() || !LocationUtil::address(loc).addressCountry().isEmpty()) {
                    return loc;
                }
            }
        }

        if ((*it).isReservation() && (*it).isLocationChange()) {
            // TODO make this work for transfers too
            const auto res = m_resMgr->reservation((*it).batchId());
            return LocationUtil::arrivalLocation(res);
        }

        if (it == m_elements.begin()) {
            break;
        }
    }

    return {};
}

bool TimelineModel::isDateEmpty(const QDate &date) const
{
    auto it = std::lower_bound(m_elements.begin(), m_elements.end(), date, [](const auto &lhs, auto rhs) {
        return lhs.dt.date() < rhs;
    });
    for (; it != m_elements.end(); ++it) {
        if ((*it).dt.date() == date && (*it).elementType != TimelineElement::TodayMarker) {
            return false;
        }
        if ((*it).dt.date() != date) {
            break;
        }
    }
    return true;
}
