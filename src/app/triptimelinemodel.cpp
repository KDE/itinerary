/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "triptimelinemodel.h"
#include "constants.h"
#include "locationhelper.h"
#include "locationinformation.h"
#include "reservationmanager.h"
#include "timelineelement.h"
#include "tripgroup.h"
#include "transfermanager.h"

#include "weatherforecast.h"

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
        if (!ev.endDate().isValid() || ev.startDate().date() == ev.endDate().date()) {
            return false;
        }
        // don't split single day events that end at midnight either
        if (ev.startDate().secsTo(ev.endDate()) < 60L * 60 * 8 && ev.endDate().time().hour() == 0) {
            return false;
        }
        return true;
    }

    return JsonLd::isA<LodgingReservation>(res)
        || JsonLd::isA<RentalCarReservation>(res);
}

static QTimeZone timeZone(const QDateTime &dt)
{
    return dt.timeSpec() == Qt::TimeZone ? dt.timeZone() : QTimeZone();
}

TripTimelineModel::TripTimelineModel(QObject *parent)
    : AbstractTimelineModel(parent)
{
    connect(&m_dayUpdateTimer, &QTimer::timeout, this, &TripTimelineModel::dayChanged);
    m_dayUpdateTimer.setTimerType(Qt::VeryCoarseTimer);
    m_dayUpdateTimer.setSingleShot(true);
    m_dayUpdateTimer.setInterval((QTime::currentTime().secsTo({23, 59, 59}) + 1) * 1000);
    m_dayUpdateTimer.start();

    // make sure we properly update the empty today marker
    connect(this, &TripTimelineModel::todayRowChanged, this, [this]() {
        const auto idx = index(todayRow(), 0);
        if (m_todayEmpty == idx.data(TripTimelineModel::TodayEmptyRole).toBool()) {
            return;
        }
        m_todayEmpty = !m_todayEmpty;
        Q_EMIT dataChanged(idx, idx);
    });

    connect(&m_currentBatchTimer, &QTimer::timeout, this, &TripTimelineModel::currentBatchChanged);
    connect(&m_currentBatchTimer, &QTimer::timeout, this, &TripTimelineModel::updateTodayMarker);
    connect(&m_currentBatchTimer, &QTimer::timeout, this, &TripTimelineModel::scheduleCurrentBatchTimer);
    m_currentBatchTimer.setTimerType(Qt::VeryCoarseTimer);
    m_currentBatchTimer.setSingleShot(true);
}

TripTimelineModel::~TripTimelineModel() = default;

void TripTimelineModel::setReservationManager(ReservationManager* mgr)
{
    // for auto tests only
    if (Q_UNLIKELY(!mgr)) {
        beginResetModel();
        disconnect(m_resMgr, &ReservationManager::batchAdded, this, &TripTimelineModel::batchAdded);
        disconnect(m_resMgr, &ReservationManager::batchChanged, this, &TripTimelineModel::batchChanged);
        disconnect(m_resMgr, &ReservationManager::batchContentChanged, this, &TripTimelineModel::batchChanged);
        disconnect(m_resMgr, &ReservationManager::batchRenamed, this, &TripTimelineModel::batchRenamed);
        disconnect(m_resMgr, &ReservationManager::batchRemoved, this, &TripTimelineModel::batchRemoved);
        m_resMgr = mgr;
        m_elements.clear();
        endResetModel();
        return;
    }

    if (m_resMgr == mgr) {
        return;
    }

    m_resMgr = mgr;
    updateElements();

    connect(mgr, &ReservationManager::batchAdded, this, &TripTimelineModel::batchAdded);
    connect(mgr, &ReservationManager::batchChanged, this, &TripTimelineModel::batchChanged);
    connect(mgr, &ReservationManager::batchContentChanged, this, &TripTimelineModel::batchChanged);
    connect(mgr, &ReservationManager::batchRenamed, this, &TripTimelineModel::batchRenamed);
    connect(mgr, &ReservationManager::batchRemoved, this, &TripTimelineModel::batchRemoved);

    updateTodayMarker();
    updateInformationElements();
    Q_EMIT todayRowChanged();

    scheduleCurrentBatchTimer();
    Q_EMIT currentBatchChanged();

    Q_EMIT reservationManagerChanged();
}

void TripTimelineModel::updateElements()
{
    beginResetModel();
    for (const auto &resId : m_resMgr->batches()) {
        if (!m_tripGroup.elements().contains(resId)) {
            continue;
        }
        const auto res = m_resMgr->reservation(resId);
        auto elem = TimelineElement(this, resId, res, TimelineElement::SelfContained);
        if (!elem.isReservation()) { // a type we can't handle
            continue;
        }
        if (needsSplitting(res)) {
            m_elements.emplace_back(this, resId, res, TimelineElement::RangeBegin);
            m_elements.emplace_back(this, resId, res, TimelineElement::RangeEnd);
        } else {
            m_elements.push_back(std::move(elem));
        }
    }
    m_elements.emplace_back(this, TimelineElement::TodayMarker,  QDateTime(today(), QTime(0, 0)));
    std::sort(m_elements.begin(), m_elements.end());
    endResetModel();
}

void TripTimelineModel::setHomeCountryIsoCode(const QString &isoCode)
{
    m_homeCountry = isoCode;
    updateInformationElements();
}

TransferManager *TripTimelineModel::transferManager() const
{
    return m_transferManager;
}

void TripTimelineModel::setTransferManager(TransferManager *mgr)
{
    if (m_transferManager == mgr) {
        return;
    }

    m_transferManager = mgr;
    connect(mgr, &TransferManager::transferAdded, this, &TripTimelineModel::transferChanged);
    connect(mgr, &TransferManager::transferChanged, this, &TripTimelineModel::transferChanged);
    connect(mgr, &TransferManager::transferRemoved, this, &TripTimelineModel::transferRemoved);

    // load existing transfers into the model
    for (const auto &batchId : m_resMgr->batches()) {
        updateTransfersForBatch(batchId);
    }

    Q_EMIT transferManagerChanged();
}

int TripTimelineModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_resMgr) {
        return 0;
    }
    return (int)m_elements.size();
}

QVariant TripTimelineModel::data(const QModelIndex& index, int role) const
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
    case LocationInformationRole:
        if (elem.elementType == TimelineElement::LocationInfo) {
            return elem.content();
        }
        return {};
    case ReservationsRole:
    {
        if (!elem.isReservation()) {
            return {};
        }
        const auto resIds = m_resMgr->reservationsForBatch(elem.batchId());
        QList<QVariant> v;
        v.reserve(resIds.size());
        for (const auto &resId : resIds) {
            v.push_back(m_resMgr->reservation(resId));
        }
        std::sort(v.begin(), v.end(), SortUtil::isBefore);
        return QVariant::fromValue(v);
    }
    case TransferRole:
        if (elem.elementType == TimelineElement::Transfer) {
            return elem.content();
        }
        return {};
    case StartDateTimeRole:
        return elem.dt;
    case EndDateTimeRole:
        return elem.endDateTime();
    case IsTimeboxedRole:
        return elem.isTimeBoxed();
    case IsCanceledRole:
        return elem.isCanceled();
    default:
        return {};
    }
}

QHash<int, QByteArray> TripTimelineModel::roleNames() const
{
    auto names = QAbstractListModel::roleNames();
    names.insert(SectionHeaderRole, "sectionHeader");
    names.insert(BatchIdRole, "batchId");
    names.insert(ElementTypeRole, "type");
    names.insert(TodayEmptyRole, "isTodayEmpty");
    names.insert(IsTodayRole, "isToday");
    names.insert(LocationInformationRole, "locationInformation");
    names.insert(ReservationsRole, "reservations");
    names.insert(TransferRole, "transfer");
    return names;
}

int TripTimelineModel::todayRow() const
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [](const auto &e) { return e.elementType == TimelineElement::TodayMarker; });
    return std::distance(m_elements.begin(), it);
}

void TripTimelineModel::batchAdded(const QString &resId)
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

void TripTimelineModel::insertElement(TimelineElement &&elem)
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

std::vector<TimelineElement>::iterator TripTimelineModel::insertOrUpdate(std::vector<TimelineElement>::iterator it, TimelineElement &&elem)
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

void TripTimelineModel::batchChanged(const QString &resId)
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

void TripTimelineModel::batchRenamed(const QString& oldBatchId, const QString& newBatchId)
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

void TripTimelineModel::updateElement(const QString &resId, const QVariant &res, TimelineElement::RangeType rangeType)
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

void TripTimelineModel::batchRemoved(const QString &resId)
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

void TripTimelineModel::dayChanged()
{
    updateTodayMarker();

    m_dayUpdateTimer.setInterval((QTime::currentTime().secsTo({23, 59, 59}) + 1) * 1000);
    m_dayUpdateTimer.start();

    scheduleCurrentBatchTimer();
    Q_EMIT currentBatchChanged();
}

void TripTimelineModel::updateTodayMarker()
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

void TripTimelineModel::updateInformationElements()
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

    // add DST transition information in a -1/+3 months window, if there are any
    if (!m_elements.empty()) {
        qDebug() << "looking for DST transitions";
        constexpr const auto DST_DAYS_BEFORE = 30;
        constexpr const auto DST_DAYS_AFTER = 90;
        for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
            if ((*it).isInformational() || (*it).isCanceled()) {
                continue;
            }
            const auto tz = timeZone((*it).endDateTime());
            auto startDt = std::max((*it).dt, now().addDays(-DST_DAYS_BEFORE));
            auto endDt = now().addDays(DST_DAYS_AFTER);
            for (auto nextIt = it; std::next(nextIt) != m_elements.end();) {
                ++nextIt;
                if ((*nextIt).isInformational() || (*nextIt).isCanceled()) {
                    continue;
                }
                endDt = std::min(endDt, (*nextIt).dt);
                break;
            }
            if (startDt > now().addDays(DST_DAYS_AFTER) || endDt < now().addDays(-DST_DAYS_BEFORE) || !tz.isValid() || !tz.hasTransitions() || !tz.hasDaylightTime()) {
                continue;
            }
            while (startDt < endDt) {
                const auto nextTransition = tz.nextTransition(startDt);
                if (!nextTransition.atUtc.isValid() || nextTransition.atUtc >= endDt) {
                    break;
                }
                startDt = nextTransition.atUtc;
                LocationInformation locInfo;
                locInfo.setTimeZone(tz, startDt);
                it = insertOrUpdate(it, TimelineElement(this, TimelineElement::LocationInfo, startDt, QVariant::fromValue(locInfo)));
            }
        }

        qDebug() << "  done";
    }
}

void TripTimelineModel::updateTransfersForBatch(const QString& batchId)
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

QDateTime TripTimelineModel::now() const
{
    if (Q_UNLIKELY(m_unitTestTime.isValid())) {
        return m_unitTestTime;
    }
    return QDateTime::currentDateTime();
}

QDate TripTimelineModel::today() const
{
    if (Q_UNLIKELY(m_unitTestTime.isValid())) {
        return m_unitTestTime.date();
    }
    return QDate::currentDate();
}

void TripTimelineModel::setCurrentDateTime(const QDateTime &dt)
{
    const auto dayDiffers = today() != dt.date();
    m_unitTestTime = dt;
    if (dayDiffers && !m_elements.empty()) {
        dayChanged();
    }

    scheduleCurrentBatchTimer();
}

void TripTimelineModel::transferChanged(const Transfer& transfer)
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

void TripTimelineModel::transferRemoved(const QString &resId, Transfer::Alignment alignment)
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

[[nodiscard]] static bool isSelectableElement(const TimelineElement &elem)
{
    return elem.isReservation() && elem.rangeType == TimelineElement::SelfContained;
}

// same as SortUtil::endDateTime but with a lower bound estimate
// when the end time isn't available
[[nodiscard]] static QDateTime estimatedEndTime(const QVariant &res)
{
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

QString TripTimelineModel::currentBatchId() const
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

    auto resId = (*it).batchId();
    const auto res = m_resMgr->reservation(resId);
    auto endTime = estimatedEndTime(res);
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

void TripTimelineModel::scheduleCurrentBatchTimer()
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

QVariant TripTimelineModel::locationAtTime(const QDateTime& dt) const
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

bool TripTimelineModel::isDateEmpty(const QDate &date) const
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

ReservationManager *TripTimelineModel::reservationManager() const
{
    return m_resMgr;
}

TripGroupManager* TripTimelineModel::tripGroupManager() const
{
    return m_tripGroupManager;
}

void TripTimelineModel::setTripGroupManager(TripGroupManager *tripGroupManager)
{
    if (m_tripGroupManager == tripGroupManager) {
        return;
    }
    m_tripGroupManager = tripGroupManager;
    Q_EMIT tripGroupManagerChanged();
}

TripGroup TripTimelineModel::tripGroup() const
{
    return m_tripGroup;
}

void TripTimelineModel::setTripGroup(const TripGroup &tripGroup)
{
    if (m_tripGroup.slugName() == tripGroup.slugName()
            && m_tripGroup.elements() == tripGroup.elements()) {
        return;
    }
    m_tripGroup = tripGroup;
    Q_EMIT tripGroupChanged();

    if (m_resMgr) {
        updateElements();
    }
}

#include "moc_triptimelinemodel.cpp"
