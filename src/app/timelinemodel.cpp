/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "timelinemodel.h"
#include "locationhelper.h"
#include "locationinformation.h"
#include "pkpassmanager.h"
#include "reservationmanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"
#include "transfermanager.h"

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

#include <QDateTime>
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

static QTimeZone destinationTimeZone(const QVariant &res)
{
    const auto dt = SortUtil::endDateTime(res);
    return dt.timeSpec() == Qt::TimeZone ? dt.timeZone() : QTimeZone();
}

static GeoCoordinates geoCoordinate(const QVariant &res)
{
    if (LocationUtil::isLocationChange(res)) {
        return LocationUtil::geo(LocationUtil::arrivalLocation(res));
    }
    return LocationUtil::geo(LocationUtil::location(res));
}

static bool isCanceled(const QVariant &res)
{
    return JsonLd::canConvert<Reservation>(res) && JsonLd::convert<Reservation>(res).reservationStatus() == Reservation::ReservationCancelled;
}

TimelineModel::TimelineModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&m_dayUpdateTimer, &QTimer::timeout, this, &TimelineModel::dayChanged);
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
        auto elem = TimelineElement(resId, res, TimelineElement::SelfContained);
        if (!elem.isReservation()) { // a type we can't handle
            continue;
        }
        if (needsSplitting(res)) {
            m_elements.push_back(TimelineElement{resId, res, TimelineElement::RangeBegin});
            m_elements.push_back(TimelineElement{resId, res, TimelineElement::RangeEnd});
        } else {
            m_elements.push_back(std::move(elem));
        }
    }
    m_elements.push_back(TimelineElement{TimelineElement::TodayMarker, QDateTime(today(), QTime(0, 0))});
    std::sort(m_elements.begin(), m_elements.end());

    connect(mgr, &ReservationManager::batchAdded, this, &TimelineModel::batchAdded);
    connect(mgr, &ReservationManager::batchChanged, this, &TimelineModel::batchChanged);
    connect(mgr, &ReservationManager::batchContentChanged, this, &TimelineModel::batchChanged);
    connect(mgr, &ReservationManager::batchRenamed, this, &TimelineModel::batchRenamed);
    connect(mgr, &ReservationManager::batchRemoved, this, &TimelineModel::batchRemoved);
    endResetModel();

    updateInformationElements();
    Q_EMIT todayRowChanged();
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
        case SectionHeader:
        {
            if (elem.dt.isNull()) {
                return {};
            }
            if (elem.dt.date() == today()) {
                return i18n("Today");
            }
            return i18nc("weekday, date", "%1, %2", QLocale().dayName(elem.dt.date().dayOfWeek(), QLocale::LongFormat), QLocale().toString(elem.dt.date(), QLocale::ShortFormat));
        }
        case BatchIdRole:
            return elem.isReservation() ? elem.batchId() : QString();
        case ElementTypeRole:
            return elem.elementType;
        case TodayEmptyRole:
            if (elem.elementType == TimelineElement::TodayMarker) {
                return index.row() == (int)(m_elements.size() - 1) || m_elements.at(index.row() + 1).dt.date() > today();
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
    }
    return {};
}

QHash<int, QByteArray> TimelineModel::roleNames() const
{
    auto names = QAbstractListModel::roleNames();
    names.insert(SectionHeader, "sectionHeader");
    names.insert(BatchIdRole, "batchId");
    names.insert(ElementTypeRole, "type");
    names.insert(TodayEmptyRole, "isTodayEmpty");
    names.insert(IsTodayRole, "isToday");
    names.insert(ElementRangeRole, "rangeType");
    names.insert(LocationInformationRole, "locationInformation");
    names.insert(WeatherForecastRole, "weatherForecast");
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
        insertElement(TimelineElement{resId, res, TimelineElement::RangeBegin});
        insertElement(TimelineElement{resId, res, TimelineElement::RangeEnd});
    } else {
        insertElement(TimelineElement{resId, res, TimelineElement::SelfContained});
    }

    updateInformationElements();
    updateTransfersForBatch(resId);
    Q_EMIT todayRowChanged();
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
        insertElement(TimelineElement{resId, res, rangeType});
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
}

void TimelineModel::dayChanged()
{
    updateTodayMarker();
    updateWeatherElements();

    m_dayUpdateTimer.setInterval((QTime::currentTime().secsTo({23, 59, 59}) + 1) * 1000);
    m_dayUpdateTimer.start();
}

void TimelineModel::updateTodayMarker()
{
    const auto it = std::lower_bound(m_elements.begin(), m_elements.end(), today(), [](const auto &lhs, const auto &rhs) {
        return lhs.dt.date() < rhs;
    });
    const auto newRow = std::distance(m_elements.begin(), it);
    const auto oldRow = todayRow();
    Q_ASSERT(oldRow < newRow);

    beginInsertRows({}, newRow, newRow);
    m_elements.insert(it, TimelineElement{TimelineElement::TodayMarker, QDateTime(today(), QTime(0, 0))});
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

        if (!(*it).isReservation()) {
            ++it;
            continue;
        }
        const auto res = m_resMgr->reservation((*it).batchId());
        if (isCanceled(res)) {
            ++it;
            continue;
        }

        auto newCountry = homeCountry;
        newCountry.setIsoCode(LocationHelper::destinationCountry(res));
        newCountry.setTimeZone(previousCountry.timeZone(), (*it).dt);
        newCountry.setTimeZone(destinationTimeZone(res), (*it).dt);
        if (newCountry == previousCountry) {
            ++it;
            continue;
        }
        if (!(newCountry == homeCountry) || newCountry.hasRelevantTimeZoneChange(previousCountry)) {
            // for location changes, we want this after the corresponding element
            const auto dt =  LocationUtil::isLocationChange(res) ? SortUtil::endDateTime(res) : SortUtil::startDateTime(res);
            it = insertOrUpdate(it, TimelineElement{TimelineElement::LocationInfo, dt, QVariant::fromValue(newCountry)});
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

        if (!(*it).isReservation()) {
            ++it;
            continue;
        }
        const auto res = m_resMgr->reservation((*it).batchId());
        if (!isCanceled(res)) {
            const auto newGeo = geoCoordinate(res);
            if (LocationUtil::isLocationChange(res) || newGeo.isValid()) {
                geo = newGeo;
            }
        }

        ++it;
    }

    auto date = now();
    // round to next full hour
    date.setTime(QTime(date.time().hour(), 0));
    date = date.addSecs(60 * 60);
    const auto maxForecastTime = m_weatherMgr->maximumForecastTime(date.date());

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
            if (!(*it).isReservation()) {
                ++it;
                continue;
            }
            const auto res = m_resMgr->reservation((*it).batchId());
            if (!isCanceled(res)) {
                const auto newGeo = geoCoordinate(res);
                if (LocationUtil::isLocationChange(res) || newGeo.isValid()) {
                    geo = newGeo;
                }
            }

            ++it;
            continue;
        }

        // determine the length of the forecast range (at most until the end of the day)
        auto endTime = date;
        endTime.setTime(QTime(23, 59, 59));
        auto nextStartTime = endTime;
        GeoCoordinates newGeo = geo;
        for (auto it2 = it; it2 != m_elements.end(); ++it2) {
            if ((*it2).dt >= endTime) {
                break;
            }
            if (!(*it2).isReservation()) {
                continue;
            }
            const auto res = m_resMgr->reservation((*it2).batchId());
            if (LocationUtil::isLocationChange(res)) {
                // exclude the actual travel time from forecast ranges
                endTime = std::min(endTime, TimelineElement::relevantDateTime(res, TimelineElement::RangeBegin));
                nextStartTime = std::max(endTime, TimelineElement::relevantDateTime(res, TimelineElement::RangeEnd));
                newGeo = geoCoordinate(res);
                break;
            }
        }

        ::WeatherForecast fc;
        if (geo.isValid()) {
            m_weatherMgr->monitorLocation(geo.latitude(), geo.longitude());
            fc = m_weatherMgr->forecast(geo.latitude(), geo.longitude(), date, endTime);
        }
        geo = newGeo;

        // updated or new data
        if (fc.isValid()) {
            it = insertOrUpdate(it, TimelineElement{TimelineElement::WeatherForecast, date, QVariant::fromValue(fc)});
        }
        // we have no forecast data, but a matching weather element: remove
        else if ((*it).elementType == TimelineElement::WeatherForecast && (*it).dt == date) {
            const auto row = std::distance(m_elements.begin(), it);
            beginRemoveRows({}, row, row);
            it = m_elements.erase(it);
            endRemoveRows();
        }

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
            it = m_elements.insert(it, TimelineElement{TimelineElement::WeatherForecast, date, QVariant::fromValue(fc)});
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
}

void TimelineModel::tripGroupAdded(const QString& groupId)
{
    const auto g = m_tripGroupManager->tripGroup(groupId);

    TimelineElement beginElem{TimelineElement::TripGroup, g.beginDateTime(), groupId};
    beginElem.rangeType = TimelineElement::RangeBegin;
    insertElement(std::move(beginElem));

    TimelineElement endElem{TimelineElement::TripGroup, g.endDateTime(), groupId};
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
    insertOrUpdate(it, TimelineElement(transfer));

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
