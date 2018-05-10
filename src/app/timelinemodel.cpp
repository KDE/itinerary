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
#include "pkpassmanager.h"
#include "reservationmanager.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Organization>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <KPkPass/Pass>

#include <KLocalizedString>

#include <QDateTime>
#include <QDebug>
#include <QLocale>

using namespace KItinerary;

static QDateTime relevantDateTime(const QVariant &res, TimelineModel::RangeType range)
{
    if (res.isNull()) { // today marker
        return QDateTime(QDate::currentDate(), QTime(0, 0));
    } else if (res.userType() == qMetaTypeId<FlightReservation>()) {
        const auto flight = res.value<FlightReservation>().reservationFor().value<Flight>();
        if (flight.boardingTime().isValid()) {
            return flight.boardingTime();
        }
        if (flight.departureTime().isValid()) {
            return flight.departureTime();
        }
        return QDateTime(flight.departureDay(), QTime(23, 59, 59));
    } else if (res.userType() == qMetaTypeId<TrainReservation>()) {
        return res.value<TrainReservation>().reservationFor().value<TrainTrip>().departureTime();
    } else if (res.userType() == qMetaTypeId<BusReservation>()) {
        return res.value<BusReservation>().reservationFor().value<BusTrip>().departureTime();
    } else if (res.userType() == qMetaTypeId<FoodEstablishmentReservation>()) {
        return res.value<FoodEstablishmentReservation>().startTime();
    } else if (res.userType() == qMetaTypeId<LodgingReservation>()) {
        const auto hotel = res.value<LodgingReservation>();
        // hotel checkin/checkout is always considered the first/last thing of the day
        if (range == TimelineModel::RangeEnd) {
            return QDateTime(hotel.checkoutTime().date(), QTime(0, 0, 0));
        } else {
            return QDateTime(hotel.checkinTime().date(), QTime(23, 59, 59));
        }
    }

    return {};
}

static QString passId(const QVariant &res)
{
    const auto passTypeId = JsonLdDocument::readProperty(res, "pkpassPassTypeIdentifier").toString();
    const auto serialNum = JsonLdDocument::readProperty(res, "pkpassSerialNumber").toString();
    if (passTypeId.isEmpty() || serialNum.isEmpty())
        return {};
    return passTypeId + QLatin1Char('/') + QString::fromUtf8(serialNum.toUtf8().toBase64(QByteArray::Base64UrlEncoding));
}

TimelineModel::TimelineModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

TimelineModel::~TimelineModel() = default;

void TimelineModel::setPkPassManager(PkPassManager* mgr)
{
    m_passMgr = mgr;
}

void TimelineModel::setReservationManager(ReservationManager* mgr)
{
    beginResetModel();
    m_resMgr = mgr;
    for (const auto &resId : mgr->reservations()) {
        m_elements.push_back(Element{resId, relevantDateTime(mgr->reservation(resId), SelfContained), SelfContained});
    }
    m_elements.push_back(Element{{}, relevantDateTime({}, SelfContained), SelfContained}); // today marker
    std::sort(m_elements.begin(), m_elements.end(), [this](const Element &lhs, const Element &rhs) {
        return lhs.dt < rhs.dt;
    });
    connect(mgr, &ReservationManager::reservationAdded, this, &TimelineModel::reservationAdded);
    connect(mgr, &ReservationManager::reservationUpdated, this, &TimelineModel::reservationUpdated);
    connect(mgr, &ReservationManager::reservationRemoved, this, &TimelineModel::reservationRemoved);
    endResetModel();
    emit todayRowChanged();
}

int TimelineModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_resMgr)
        return 0;
    return m_elements.size();
}

QVariant TimelineModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_resMgr)
        return {};

    const auto &elem = m_elements.at(index.row());
    const auto res = m_resMgr->reservation(elem.id);
    switch (role) {
        case PassRole:
            return QVariant::fromValue(m_passMgr->pass(passId(res)));
        case PassIdRole:
            return passId(res);
        case SectionHeader:
        {
            if (elem.dt.isNull())
                return {};
            if (elem.dt.date() == QDate::currentDate())
                return i18n("Today");
            return i18nc("weekday, date", "%1, %2", QLocale().dayName(elem.dt.date().dayOfWeek(), QLocale::LongFormat), QLocale().toString(elem.dt.date(), QLocale::ShortFormat));
        }
        case ReservationRole:
            return res;
        case ElementTypeRole:
            if (res.isNull())
                return TodayMarker;
            else if (res.userType() == qMetaTypeId<FlightReservation>())
                return Flight;
            else if (res.userType() == qMetaTypeId<LodgingReservation>())
                return Hotel;
            else if (res.userType() == qMetaTypeId<TrainReservation>())
                return TrainTrip;
            else if (res.userType() == qMetaTypeId<BusReservation>())
                return BusTrip;
            else if (res.userType() == qMetaTypeId<FoodEstablishmentReservation>())
                return Restaurant;
            return {};
        case TodayEmptyRole:
            if (res.isNull()) {
                return index.row() == (int)(m_elements.size() - 1) || m_elements.at(index.row() + 1).dt.date() > QDate::currentDate();
            }
            return {};
        case IsTodayRole:
            return elem.dt.date() == QDate::currentDate();
        case ElementRangeRole:
            return elem.rangeType;
    }
    return {};
}

QHash<int, QByteArray> TimelineModel::roleNames() const
{
    auto names = QAbstractListModel::roleNames();
    names.insert(PassRole, "pass");
    names.insert(PassIdRole, "passId");
    names.insert(SectionHeader, "sectionHeader");
    names.insert(ReservationRole, "reservation");
    names.insert(ElementTypeRole, "type");
    names.insert(TodayEmptyRole, "isTodayEmpty");
    names.insert(IsTodayRole, "isToday");
    return names;
}

int TimelineModel::todayRow() const
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [](const Element &e) { return e.id.isEmpty(); });
    return std::distance(m_elements.begin(), it);
}

void TimelineModel::reservationAdded(const QString &resId)
{
    auto it = std::lower_bound(m_elements.begin(), m_elements.end(), resId, [this](const Element &lhs, const QString &rhs) {
        return lhs.dt < relevantDateTime(m_resMgr->reservation(rhs), SelfContained);
    });
    auto index = std::distance(m_elements.begin(), it);
    beginInsertRows({}, index, index);
    m_elements.insert(it, Element{resId, relevantDateTime(m_resMgr->reservation(resId), SelfContained), SelfContained});
    endInsertRows();
    emit todayRowChanged();
}

void TimelineModel::reservationUpdated(const QString &resId)
{
    auto it = std::lower_bound(m_elements.begin(), m_elements.end(), resId, [this](const Element &lhs, const QString &rhs) {
        return lhs.dt < relevantDateTime(m_resMgr->reservation(rhs), SelfContained);
    });
    auto row = std::distance(m_elements.begin(), it);
    emit dataChanged(index(row, 0), index(row, 0));
}

void TimelineModel::reservationRemoved(const QString &resId)
{
    const auto it = std::find_if(m_elements.begin(), m_elements.end(), [resId](const Element &e) { return e.id == resId; });
    if (it == m_elements.end()) {
        return;
    }
    const auto row = std::distance(m_elements.begin(), it);
    beginRemoveRows({}, row, row);
    m_elements.erase(it);
    endRemoveRows();
    emit todayRowChanged();
}
