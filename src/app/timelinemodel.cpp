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

#include <weatherforecastmanager.h>

#include <KItinerary/BusTrip>
#include <KItinerary/CountryDb>
#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
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

static QString passId(const QVariant &res)
{
    const auto passTypeId = JsonLdDocument::readProperty(res, "pkpassPassTypeIdentifier").toString();
    const auto serialNum = JsonLdDocument::readProperty(res, "pkpassSerialNumber").toString();
    if (passTypeId.isEmpty() || serialNum.isEmpty()) {
        return {};
    }
    return passTypeId + QLatin1Char('/') + QString::fromUtf8(serialNum.toUtf8().toBase64(QByteArray::Base64UrlEncoding));
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

TimelineModel::TimelineModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

TimelineModel::~TimelineModel() = default;

void TimelineModel::setReservationManager(ReservationManager* mgr)
{
    beginResetModel();
    m_resMgr = mgr;
    for (const auto &resId : mgr->reservations()) {
        const auto res = m_resMgr->reservation(resId);
        if (needsSplitting(res)) {
            m_elements.push_back(Element{resId, {}, relevantDateTime(res, RangeBegin), elementType(res), RangeBegin});
            m_elements.push_back(Element{resId, {}, relevantDateTime(res, RangeEnd), elementType(res), RangeEnd});
        } else {
            m_elements.push_back(Element{resId, {}, relevantDateTime(res, SelfContained), elementType(res), SelfContained});
        }
    }
    m_elements.push_back(Element{{}, {}, QDateTime(QDate::currentDate(), QTime(0, 0)), TodayMarker, SelfContained});
    std::sort(m_elements.begin(), m_elements.end(), [](const Element &lhs, const Element &rhs) {
        return lhs.dt < rhs.dt;
    });
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
    insertWeatherElements();
    connect(m_weatherMgr, &WeatherForecastManager::forecastUpdated, this, &TimelineModel::updateWeatherElements);
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
    const auto res = m_resMgr->reservation(elem.id);
    switch (role) {
        case PassIdRole:
            return passId(res);
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
            return elem.id;
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
    names.insert(PassIdRole, "passId");
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
        insertElement(Element{resId, {}, relevantDateTime(res, RangeBegin), elementType(res), RangeBegin});
        insertElement(Element{resId, {}, relevantDateTime(res, RangeEnd), elementType(res), RangeEnd});
    } else {
        insertElement(Element{resId, {}, relevantDateTime(res, SelfContained), elementType(res), SelfContained});
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
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [resId, rangeType](const Element &e) { return e.id == resId && e.rangeType == rangeType; });
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
        insertElement(Element{resId, {}, newDt, elementType(res), rangeType});
    } else {
        emit dataChanged(index(row, 0), index(row, 0));
    }
}

void TimelineModel::reservationRemoved(const QString &resId)
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [resId](const Element &e) { return e.id == resId; });
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
    homeCountry.setIsoCode(QLatin1String("DE")); // TODO configurable home country

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
        newCountry.setIsoCode(destinationCountry(m_resMgr->reservation((*it).id)));
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
        it = m_elements.insert(it, Element{{}, QVariant::fromValue(newCountry), (*it).dt, CountryInfo, SelfContained});
        endInsertRows();

        previousCountry = newCountry;
    }

    insertWeatherElements();
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

void TimelineModel::insertWeatherElements()
{
    if (!m_weatherMgr) {
        return;
    }

    auto it = std::find_if(m_elements.begin(), m_elements.end(), [](const Element &e) { return e.elementType == TodayMarker; });
    auto date = QDate::currentDate();
    for (; it != m_elements.end() && date < QDate::currentDate().addDays(9);) {
        if ((*it).dt.date() < date || (*it).elementType == TodayMarker) {
            ++it;
            continue;
        }
        if (date == (*it).dt.date() && (*it).elementType == WeatherForecast) { // weather element already present
            date = date.addDays(1);
            ++it;
            continue;
        }

        const auto geo = geoCoordinate(it);
        if (geo.isValid()) {
            m_weatherMgr->monitorLocation(geo.latitude(), geo.longitude());
            const auto fc = m_weatherMgr->forecast(geo.latitude(), geo.longitude(), QDateTime(date, QTime(12, 0)));
            if (fc.isValid()) {
                const auto row = std::distance(m_elements.begin(), it);
                beginInsertRows({}, row, row);
                it = m_elements.insert(it, Element{{}, fc, QDateTime(date, QTime()), WeatherForecast, SelfContained});
                endInsertRows();
                date = date.addDays(1);
                continue;
            }
        }
        date = date.addDays(1);
        ++it;
    }
}

void TimelineModel::updateWeatherElements()
{
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
        if ((*it).elementType == WeatherForecast) {
            // TODO see above
            (*it).content = m_weatherMgr->forecast(52, 13.5, QDateTime((*it).dt.date(), QTime(12, 0)));
            const auto idx = index(std::distance(m_elements.begin(), it), 0);
            emit dataChanged(idx, idx);
        }
    }

    insertWeatherElements();
}

GeoCoordinates TimelineModel::geoCoordinate(std::vector<Element>::iterator it) const
{
    if (it == m_elements.begin()) {
        return {};
    }
    --it;

    do {
        if ((*it).id.isEmpty()) {
            --it;
            continue;
        }

        const auto res = m_resMgr->reservation((*it).id);
        // things that change location
        if (JsonLd::isA<FlightReservation>(res)) {
            return res.value<FlightReservation>().reservationFor().value<KItinerary::Flight>().arrivalAirport().geo();
        }
        if (JsonLd::isA<TrainReservation>(res)) {
            return res.value<TrainReservation>().reservationFor().value<KItinerary::TrainTrip>().arrivalStation().geo();
        }
        if (JsonLd::isA<BusReservation>(res)) {
            return res.value<BusReservation>().reservationFor().value<KItinerary::BusTrip>().arrivalStation().geo();
        }

        // things that don't change location
        GeoCoordinates geo;
        if (JsonLd::isA<LodgingReservation>(res)) {
            geo = res.value<LodgingReservation>().reservationFor().value<LodgingBusiness>().geo();
        }
        if (JsonLd::isA<FoodEstablishmentReservation>(res)) {
            geo = res.value<FoodEstablishmentReservation>().reservationFor().value<FoodEstablishment>().geo();
        }
        if (JsonLd::isA<TouristAttractionVisit>(res)) {
            geo = res.value<TouristAttractionVisit>().touristAttraction().geo();
        }
        if (geo.isValid()) {
            return geo;
        }

        --it;
    } while (it != m_elements.begin());

    return {};
}
