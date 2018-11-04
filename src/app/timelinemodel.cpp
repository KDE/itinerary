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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "timelinemodel.h"
#include "countryinformation.h"
#include "pkpassmanager.h"
#include "reservationmanager.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"

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
    return JsonLd::isA<LodgingReservation>(res)
        || JsonLd::isA<RentalCarReservation>(res);
}

static QDateTime relevantDateTime(const QVariant &res, TimelineModel::RangeType range)
{
    if (range == TimelineModel::RangeBegin || range == TimelineModel::SelfContained) {
        return SortUtil::startDateTime(res);
    }
    if (range == TimelineModel::RangeEnd) {
        return SortUtil::endtDateTime(res);
    }

    return {};
}

static TimelineModel::ElementType elementType(const QVariant &res)
{
    if (JsonLd::isA<FlightReservation>(res)) { return TimelineModel::Flight; }
    if (JsonLd::isA<LodgingReservation>(res)) { return TimelineModel::Hotel; }
    if (JsonLd::isA<TrainReservation>(res)) { return TimelineModel::TrainTrip; }
    if (JsonLd::isA<BusReservation>(res)) { return TimelineModel::BusTrip; }
    if (JsonLd::isA<FoodEstablishmentReservation>(res)) { return TimelineModel::Restaurant; }
    if (JsonLd::isA<TouristAttractionVisit>(res)) { return TimelineModel::TouristAttraction; }
    if (JsonLd::isA<EventReservation>(res)) { return TimelineModel::Event; }
    if (JsonLd::isA<RentalCarReservation>(res)) { return TimelineModel::CarRental; }
    return {};
}

static QString destinationCountry(const QVariant &res)
{
    if (LocationUtil::isLocationChange(res)) {
        return LocationUtil::address(LocationUtil::arrivalLocation(res)).addressCountry();
    }
    return LocationUtil::address(LocationUtil::location(res)).addressCountry();
}

static GeoCoordinates geoCoordinate(const QVariant &res)
{
    if (LocationUtil::isLocationChange(res)) {
        return LocationUtil::geo(LocationUtil::arrivalLocation(res));
    }
    return LocationUtil::geo(LocationUtil::location(res));
}


TimelineModel::Element::Element(TimelineModel::ElementType type, const QDateTime &dateTime, const QVariant &data)
    : content(data)
    , dt(dateTime)
    , elementType(type)
{
}

TimelineModel::Element::Element(const QString& resId, const QVariant& res, RangeType rt)
    : dt(relevantDateTime(res, rt))
    , elementType(::elementType(res))
    , rangeType(rt)
{
    ids.push_back(resId);
}

TimelineModel::TimelineModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&m_dayUpdateTimer, &QTimer::timeout, this, &TimelineModel::dayChanged);
    m_dayUpdateTimer.setSingleShot(true);
    m_dayUpdateTimer.setInterval((QTime::currentTime().secsTo({23, 59, 59}) + 1) * 1000);
    m_dayUpdateTimer.start();
}

TimelineModel::~TimelineModel() = default;

void TimelineModel::setReservationManager(ReservationManager* mgr)
{
    // for auto tests only
    if (Q_UNLIKELY(!mgr)) {
        beginResetModel();
        disconnect(m_resMgr, &ReservationManager::reservationAdded, this, &TimelineModel::reservationAdded);
        disconnect(m_resMgr, &ReservationManager::reservationUpdated, this, &TimelineModel::reservationUpdated);
        disconnect(m_resMgr, &ReservationManager::reservationRemoved, this, &TimelineModel::reservationRemoved);
        m_resMgr = mgr;
        m_elements.clear();
        endResetModel();
        return;
    }

    beginResetModel();
    m_resMgr = mgr;
    for (const auto &resId : mgr->reservations()) {
        const auto res = m_resMgr->reservation(resId);
        if (needsSplitting(res)) {
            m_elements.push_back(Element{resId, res, RangeBegin});
            m_elements.push_back(Element{resId, res, RangeEnd});
        } else {
            m_elements.push_back(Element{resId, res, SelfContained});
        }
    }
    m_elements.push_back(Element{TodayMarker, QDateTime(today(), QTime(0, 0))});
    std::sort(m_elements.begin(), m_elements.end(), elementLessThan);

    // merge multi-traveler elements
    QDateTime prevDt;
    for (auto it = m_elements.begin(); it != m_elements.end();) {
        if ((*it).dt != prevDt || !prevDt.isValid()) {
            prevDt = (*it).dt;
            ++it;
            continue;
        }
        prevDt = (*it).dt;
        auto prevIt = it - 1;

        if ((*prevIt).rangeType != (*it).rangeType || (*prevIt).elementType != (*it).elementType || (*prevIt).ids.isEmpty() || (*it).ids.isEmpty()) {
            ++it;
            continue;
        }

        const auto prevRes = m_resMgr->reservation((*prevIt).ids.at(0));
        const auto curRes = m_resMgr->reservation((*it).ids.at(0));
        if (prevRes.isNull() || curRes.isNull() || prevRes.userType() != curRes.userType() || !JsonLd::canConvert<Reservation>(prevRes)) {
            ++it;
            continue;
        }

        const auto prevTrip = JsonLd::convert<Reservation>(prevRes).reservationFor();
        const auto curTrip = JsonLd::convert<Reservation>(curRes).reservationFor();
        if (MergeUtil::isSame(prevTrip, curTrip)) {
            Q_ASSERT((*it).ids.size() == 1);
            (*prevIt).ids.push_back((*it).ids.at(0));
            it = m_elements.erase(it);
        } else {
            ++it;
        }
    }

    connect(mgr, &ReservationManager::reservationAdded, this, &TimelineModel::reservationAdded);
    connect(mgr, &ReservationManager::reservationUpdated, this, &TimelineModel::reservationUpdated);
    connect(mgr, &ReservationManager::reservationRemoved, this, &TimelineModel::reservationRemoved);
    endResetModel();

    updateInformationElements();
    emit todayRowChanged();
}

void TimelineModel::setWeatherForecastManager(WeatherForecastManager* mgr)
{
    m_weatherMgr = mgr;
    updateWeatherElements();
    connect(m_weatherMgr, &WeatherForecastManager::forecastUpdated, this, &TimelineModel::updateWeatherElements);
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
    const auto res = m_resMgr->reservation(elem.ids.value(0));
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
        case ReservationIdsRole:
            return elem.ids;
        case ElementTypeRole:
            return elem.elementType;
        case TodayEmptyRole:
            if (elem.elementType == TodayMarker) {
                return index.row() == (int)(m_elements.size() - 1) || m_elements.at(index.row() + 1).dt.date() > today();
            }
            return {};
        case IsTodayRole:
            return elem.dt.date() == today();
        case ElementRangeRole:
            return elem.rangeType;
        case CountryInformationRole:
            if (elem.elementType == CountryInfo)
                return elem.content;
            break;
        case WeatherForecastRole:
            if (elem.elementType == WeatherForecast)
                return elem.content;
            break;
        case ReservationsRole:
        {
            QVector<QVariant> v;
            for (const auto &resId : elem.ids)
                v.push_back(m_resMgr->reservation(resId));
            std::sort(v.begin(), v.end(), SortUtil::isBefore);
            return QVariant::fromValue(v);
        }
        case TripGroupRole:
            if (elem.elementType == TripGroup)
                return QVariant::fromValue(m_tripGroupManager->tripGroup(elem.content.toString()));
            break;
    }
    return {};
}

QHash<int, QByteArray> TimelineModel::roleNames() const
{
    auto names = QAbstractListModel::roleNames();
    names.insert(SectionHeader, "sectionHeader");
    names.insert(ReservationIdsRole, "reservationIds");
    names.insert(ElementTypeRole, "type");
    names.insert(TodayEmptyRole, "isTodayEmpty");
    names.insert(IsTodayRole, "isToday");
    names.insert(ElementRangeRole, "rangeType");
    names.insert(CountryInformationRole, "countryInformation");
    names.insert(WeatherForecastRole, "weatherForecast");
    names.insert(ReservationsRole, "reservations");
    names.insert(TripGroupRole, "tripGroup");
    return names;
}

int TimelineModel::todayRow() const
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [](const Element &e) { return e.elementType == TodayMarker; });
    return std::distance(m_elements.begin(), it);
}

bool TimelineModel::elementLessThan(const TimelineModel::Element &lhs, const TimelineModel::Element &rhs)
{
    if (lhs.dt == rhs.dt) {
        return lhs.elementType < rhs.elementType;
    }
    return lhs.dt < rhs.dt;
}

void TimelineModel::reservationAdded(const QString &resId)
{
    const auto res = m_resMgr->reservation(resId);
    if (needsSplitting(res)) {
        insertElement(Element{resId, res, RangeBegin});
        insertElement(Element{resId, res, RangeEnd});
    } else {
        insertElement(Element{resId, res, SelfContained});
    }

    updateInformationElements();
    emit todayRowChanged();
}

void TimelineModel::insertElement(Element &&elem)
{
    auto it = std::lower_bound(m_elements.begin(), m_elements.end(), elem, elementLessThan);
    const auto row = std::distance(m_elements.begin(), it);

    // check if we can merge with an existing element
    if (it != m_elements.end() && (*it).dt == elem.dt && elem.ids.size() == 1 && (*it).elementType == elem.elementType && (*it).rangeType == elem.rangeType && !(*it).ids.isEmpty()) {
        const auto prevRes = m_resMgr->reservation((*it).ids.at(0));
        const auto curRes = m_resMgr->reservation(elem.ids.at(0));
        if (prevRes.userType() == curRes.userType() && !prevRes.isNull() && !curRes.isNull() && JsonLd::canConvert<Reservation>(prevRes)) {
            const auto prevTrip = JsonLd::convert<Reservation>(prevRes).reservationFor();
            const auto curTrip = JsonLd::convert<Reservation>(curRes).reservationFor();
            if (MergeUtil::isSame(prevTrip, curTrip)) {
                (*it).ids.push_back(elem.ids.at(0));
                emit dataChanged(index(row, 0), index(row, 0));
                return;
            }
        }
    }

    beginInsertRows({}, row, row);
    m_elements.insert(it, std::move(elem));
    endInsertRows();
}

void TimelineModel::reservationUpdated(const QString &resId)
{
    const auto res = m_resMgr->reservation(resId);
    if (needsSplitting(res)) {
        updateElement(resId, res, RangeBegin);
        updateElement(resId, res, RangeEnd);
    } else {
        updateElement(resId, res, SelfContained);
    }

    updateInformationElements();
}

void TimelineModel::updateElement(const QString &resId, const QVariant &res, TimelineModel::RangeType rangeType)
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [resId, rangeType](const Element &e) { return e.ids.contains(resId) && e.rangeType == rangeType; });
    if (it == m_elements.end()) {
        return;
    }
    const auto row = std::distance(m_elements.begin(), it);
    const auto newDt = relevantDateTime(res, rangeType);
    const auto isMulti = (*it).ids.size() > 1;

    if ((*it).dt != newDt) {
        // element moved
        if (isMulti) {
            (*it).ids.removeAll(resId);
            emit dataChanged(index(row, 0), index(row, 0));
        } else {
            beginRemoveRows({}, row, row);
            m_elements.erase(it);
            endRemoveRows();
        }
        insertElement(Element{resId, res, rangeType});
    } else {
        emit dataChanged(index(row, 0), index(row, 0));
    }
}

void TimelineModel::reservationRemoved(const QString &resId)
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [resId](const Element &e) { return e.ids.contains(resId); });
    if (it == m_elements.end()) {
        return;
    }
    const auto isSplit = (*it).rangeType == RangeBegin;
    const auto row = std::distance(m_elements.begin(), it);
    const auto isMulti = (*it).ids.size() > 1;

    if (isMulti) {
        (*it).ids.removeAll(resId);
        emit dataChanged(index(row, 0), index(row, 0));
    } else {
        beginRemoveRows({}, row, row);
        m_elements.erase(it);
        endRemoveRows();
        emit todayRowChanged();
    }

    if (isSplit) {
        reservationRemoved(resId);
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
    m_elements.insert(it, Element{TodayMarker, QDateTime(today(), QTime(0, 0))});
    endInsertRows();

    beginRemoveRows({}, oldRow, oldRow);
    m_elements.erase(m_elements.begin() + oldRow);
    endRemoveRows();
}

void TimelineModel::updateInformationElements()
{
    // the country information is shown before transitioning into a country that
    // differs in one or more properties from the home country and we where that
    // differences is introduced by the transition

    CountryInformation homeCountry;
    homeCountry.setIsoCode(m_homeCountry);

    auto previousCountry = homeCountry;
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
        switch ((*it).elementType) {
            case TodayMarker:
            case WeatherForecast:
                it = erasePreviousCountyInfo(it);
                continue;
            case CountryInfo:
                previousCountry = (*it).content.value<CountryInformation>();
                it = erasePreviousCountyInfo(it); // purge multiple consecutive country info elements
                continue;
            default:
                break;
        }

        auto newCountry = homeCountry;
        newCountry.setIsoCode(destinationCountry(m_resMgr->reservation((*it).ids.value(0))));
        if (newCountry == previousCountry) {
            continue;
        }
        if (newCountry == homeCountry) {
            assert(it != m_elements.begin()); // previousCountry == homeCountry in this case
            // purge outdated country info element
            it = erasePreviousCountyInfo(it);
            previousCountry = newCountry;
            continue;
        }

        // add new country info element
        auto row = std::distance(m_elements.begin(), it);
        beginInsertRows({}, row, row);
        it = m_elements.insert(it, Element{CountryInfo, (*it).dt, QVariant::fromValue(newCountry)});
        endInsertRows();

        previousCountry = newCountry;
    }

    updateWeatherElements();
}

std::vector<TimelineModel::Element>::iterator TimelineModel::erasePreviousCountyInfo(std::vector<Element>::iterator it)
{
    if (it == m_elements.begin()) {
        return it;
    }

    auto it2 = it;
    --it2;
    if ((*it2).elementType == CountryInfo) {
        const auto row = std::distance(m_elements.begin(), it2);
        beginRemoveRows({}, row, row);
        it = m_elements.erase(it2);
        endRemoveRows();
    }
    return it;
}

void TimelineModel::updateWeatherElements()
{
    if (!m_weatherMgr || m_elements.empty()) {
        return;
    }

    qDebug() << "recomputing weather elements";
    GeoCoordinates geo;

    // look through the past, clean up weather elements there and figure out where we are
    auto it = m_elements.begin();
    for (; it != m_elements.end() && (*it).dt < now();) {
        if ((*it).elementType == WeatherForecast) {
            const auto row = std::distance(m_elements.begin(), it);
            beginRemoveRows({}, row, row);
            it = m_elements.erase(it);
            endRemoveRows();
            continue;
        }

        const auto res = m_resMgr->reservation((*it).ids.value(0));
        const auto newGeo = geoCoordinate(res);
        if (LocationUtil::isLocationChange(res) || newGeo.isValid()) {
            geo = newGeo;
        }

        ++it;
    }

    auto date = now();
    date.setTime(QTime(date.time().hour() + 1, 0)); // ### this looks suspicious for 23:xx?
    while(it != m_elements.end() && date < m_weatherMgr->maximumForecastTime(today())) {

        if ((*it).dt < date || (*it).elementType == TodayMarker) {
            // clean up outdated weather elements (happens when merging previously split ranges)
            if ((*it).elementType == WeatherForecast) {
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
            const auto res = m_resMgr->reservation((*it).ids.value(0));
            const auto newGeo = geoCoordinate(res);
            if (LocationUtil::isLocationChange(res) || newGeo.isValid()) {
                geo = newGeo;
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
            const auto res = m_resMgr->reservation((*it2).ids.value(0));
            if (LocationUtil::isLocationChange(res)) {
                // exclude the actual travel time from forecast ranges
                endTime = std::min(endTime, relevantDateTime(res, RangeBegin));
                nextStartTime = std::max(endTime, relevantDateTime(res, RangeEnd));
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

        // case 1: we have forecast data, and a matching weather element: update
        if (fc.isValid() && (*it).dt == date && (*it).elementType == WeatherForecast) {
            (*it).content = QVariant::fromValue(fc);
            const auto idx = index(std::distance(m_elements.begin(), it), 0);
            emit dataChanged(idx, idx);
        }
        // case 2: we have forecast data, but no matching weather element: insert
        else if (fc.isValid()) {
            const auto row = std::distance(m_elements.begin(), it);
            beginInsertRows({}, row, row);
            it = m_elements.insert(it, Element{WeatherForecast, date, QVariant::fromValue(fc)});
            endInsertRows();
        }
        // case 3: we have no forecast data, but a matching weather element: remove
        else if ((*it).elementType == WeatherForecast && (*it).dt == date) {
            const auto row = std::distance(m_elements.begin(), it);
            beginRemoveRows({}, row, row);
            it = m_elements.erase(it);
            endRemoveRows();
        }

        date = nextStartTime.addSecs(1);
        ++it;
    }

    // append weather elements beyond the end of the list if necessary
    while (date < m_weatherMgr->maximumForecastTime(today()) && geo.isValid()) {
        auto endTime = date;
        endTime.setTime(QTime(23, 59, 59));

        m_weatherMgr->monitorLocation(geo.latitude(), geo.longitude());
        const auto fc = m_weatherMgr->forecast(geo.latitude(), geo.longitude(), date, endTime);
        if (fc.isValid()) {
            const auto row = std::distance(m_elements.begin(), it);
            beginInsertRows({}, row, row);
            it = m_elements.insert(it, Element{WeatherForecast, date, QVariant::fromValue(fc)});
            ++it;
            endInsertRows();
        }
        date = endTime.addSecs(1);
    }

    qDebug() << "weather recomputation done";
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
    qDebug() << groupId;
    const auto g = m_tripGroupManager->tripGroup(groupId);

    Element beginElem{TimelineModel::TripGroup, g.beginDateTime(), groupId};
    beginElem.rangeType = RangeBegin;
    insertElement(std::move(beginElem));

    Element endElem{TimelineModel::TripGroup, g.endDateTime(), groupId};
    endElem.rangeType = RangeEnd;
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
    qDebug() << groupId;
    for (auto it = m_elements.begin(); it != m_elements.end();) {
        if ((*it).elementType != TripGroup || (*it).content.toString() != groupId) {
            ++it;
            continue;
        }

        const auto row = std::distance(m_elements.begin(), it);
        beginRemoveRows({}, row, row);
        it = m_elements.erase(it);
        endRemoveRows();
    }
}
