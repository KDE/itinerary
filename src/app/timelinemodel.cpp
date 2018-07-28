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

#include <weatherforecast.h>
#include <weatherforecastmanager.h>

#include <KItinerary/BusTrip>
#include <KItinerary/CountryDb>
#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
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
    return res.userType() == qMetaTypeId<LodgingReservation>();
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
    return {};
}

static QString destinationCountry(const QVariant &res)
{
    if (JsonLd::isA<FlightReservation>(res)) {
        return res.value<FlightReservation>().reservationFor().value<Flight>().arrivalAirport().address().addressCountry();
    }
    if (JsonLd::isA<TrainReservation>(res)) {
        return res.value<TrainReservation>().reservationFor().value<TrainTrip>().arrivalStation().address().addressCountry();
    }
    if (JsonLd::isA<LodgingReservation>(res)) {
        return res.value<LodgingReservation>().reservationFor().value<LodgingBusiness>().address().addressCountry();
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return res.value<BusReservation>().reservationFor().value<BusTrip>().arrivalStation().address().addressCountry();
    }
    if (JsonLd::isA<FoodEstablishmentReservation>(res)) {
        return res.value<FoodEstablishmentReservation>().reservationFor().value<FoodEstablishment>().address().addressCountry();
    }
    if (JsonLd::isA<TouristAttractionVisit>(res)) {
        return res.value<TouristAttractionVisit>().touristAttraction().address().addressCountry();
    }
    return {};
}

static GeoCoordinates geoCoordinate(const QVariant &res)
{
    if (JsonLd::isA<FlightReservation>(res)) {
        return res.value<FlightReservation>().reservationFor().value<KItinerary::Flight>().arrivalAirport().geo();
    }
    if (JsonLd::isA<TrainReservation>(res)) {
        return res.value<TrainReservation>().reservationFor().value<KItinerary::TrainTrip>().arrivalStation().geo();
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return res.value<BusReservation>().reservationFor().value<KItinerary::BusTrip>().arrivalStation().geo();
    }

    if (JsonLd::isA<LodgingReservation>(res)) {
        return res.value<LodgingReservation>().reservationFor().value<LodgingBusiness>().geo();
    }
    if (JsonLd::isA<FoodEstablishmentReservation>(res)) {
        return res.value<FoodEstablishmentReservation>().reservationFor().value<FoodEstablishment>().geo();
    }
    if (JsonLd::isA<TouristAttractionVisit>(res)) {
        return res.value<TouristAttractionVisit>().touristAttraction().geo();
    }

    return {};
}

static bool isLocationChange(const QVariant &res)
{
    return JsonLd::isA<FlightReservation>(res) || JsonLd::isA<TrainReservation>(res) || JsonLd::isA<BusReservation>(res);
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
    m_elements.push_back(Element{TodayMarker, QDateTime(QDate::currentDate(), QTime(0, 0))});
    std::sort(m_elements.begin(), m_elements.end(), [](const Element &lhs, const Element &rhs) {
        return lhs.dt < rhs.dt;
    });

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
            if (elem.dt.date() == QDate::currentDate()) {
                return i18n("Today");
            }
            return i18nc("weekday, date", "%1, %2", QLocale().dayName(elem.dt.date().dayOfWeek(), QLocale::LongFormat), QLocale().toString(elem.dt.date(), QLocale::ShortFormat));
        }
        case ReservationRole:
            return res;
        case ReservationIdRole:
            return elem.ids.value(0);
        case ElementTypeRole:
            return elem.elementType;
        case TodayEmptyRole:
            if (elem.elementType == TodayMarker) {
                return index.row() == (int)(m_elements.size() - 1) || m_elements.at(index.row() + 1).dt.date() > QDate::currentDate();
            }
            return {};
        case IsTodayRole:
            return elem.dt.date() == QDate::currentDate();
        case ElementRangeRole:
            return elem.rangeType;
        case CountryInformationRole:
        case WeatherForecastRole:
            return elem.content;
    }
    return {};
}

QHash<int, QByteArray> TimelineModel::roleNames() const
{
    auto names = QAbstractListModel::roleNames();
    names.insert(SectionHeader, "sectionHeader");
    names.insert(ReservationRole, "reservation");
    names.insert(ReservationIdRole, "reservationId");
    names.insert(ElementTypeRole, "type");
    names.insert(TodayEmptyRole, "isTodayEmpty");
    names.insert(IsTodayRole, "isToday");
    names.insert(ElementRangeRole, "rangeType");
    names.insert(CountryInformationRole, "countryInformation");
    names.insert(WeatherForecastRole, "weatherForecast");
    return names;
}

int TimelineModel::todayRow() const
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [](const Element &e) { return e.elementType == TodayMarker; });
    return std::distance(m_elements.begin(), it);
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
    auto it = std::lower_bound(m_elements.begin(), m_elements.end(), elem.dt, [](const Element &lhs, const QDateTime &rhs) {
        return lhs.dt < rhs;
    });
    auto index = std::distance(m_elements.begin(), it);
    beginInsertRows({}, index, index);
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

    if ((*it).dt != newDt) {
        // element moved
        beginRemoveRows({}, row, row);
        m_elements.erase(it);
        endRemoveRows();
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
    beginRemoveRows({}, row, row);
    m_elements.erase(it);
    endRemoveRows();
    emit todayRowChanged();

    if (isSplit) {
        reservationRemoved(resId);
    }

    updateInformationElements();
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
    for (; it != m_elements.end() && (*it).dt < QDateTime::currentDateTimeUtc();) {
        if ((*it).elementType == WeatherForecast) {
            const auto row = std::distance(m_elements.begin(), it);
            beginRemoveRows({}, row, row);
            it = m_elements.erase(it);
            endRemoveRows();
            continue;
        }

        const auto res = m_resMgr->reservation((*it).ids.value(0));
        const auto newGeo = geoCoordinate(res);
        if (isLocationChange(res) || newGeo.isValid()) {
            geo = newGeo;
        }

        ++it;
    }

    auto date = QDateTime::currentDateTime();
    date.setTime(QTime(date.time().hour() + 1, 0));
    while(it != m_elements.end() && date < m_weatherMgr->maximumForecastTime()) {

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
            if (isLocationChange(res) || newGeo.isValid()) {
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
            if (isLocationChange(res)) {
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
    while (date < m_weatherMgr->maximumForecastTime() && geo.isValid()) {
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
