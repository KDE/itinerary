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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef TIMELINEMODEL_H
#define TIMELINEMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QTimer>

class ReservationManager;
class WeatherForecastManager;
class TripGroupManager;

namespace KItinerary {
class GeoCoordinates;
}

class TimelineModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int todayRow READ todayRow NOTIFY todayRowChanged)

public:
    enum Role {
        SectionHeader = Qt::UserRole + 1,
        BatchIdRole,
        ElementTypeRole,
        TodayEmptyRole,
        IsTodayRole,
        ElementRangeRole,
        CountryInformationRole,
        WeatherForecastRole,
        ReservationsRole, // for unit testing
        TripGroupIdRole,
        TripGroupRole
    };

    // Note: the order in here defines the priority of element if they occur at the same time
    enum ElementType {
        Undefined,
        TodayMarker,
        TripGroup,
        WeatherForecast,
        CountryInfo,
        Flight,
        TrainTrip,
        CarRental,
        BusTrip,
        Restaurant,
        TouristAttraction,
        Event,
        Hotel
    };
    Q_ENUM(ElementType)

    // indicates whether an element is self-contained or the beginning/end of a longer timespan/range
    enum RangeType {
        SelfContained,
        RangeBegin,
        RangeEnd
    };
    Q_ENUM(RangeType)

    explicit TimelineModel(QObject *parent = nullptr);
    ~TimelineModel() override;

    void setReservationManager(ReservationManager *mgr);
    void setWeatherForecastManager(WeatherForecastManager *mgr);
    TripGroupManager* tripGroupManager() const;
    void setTripGroupManager(TripGroupManager *mgr);
    void setHomeCountryIsoCode(const QString &isoCode);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    int todayRow() const;

    /** Emit dataChanged() for the row containing @p resId.
     *  This is used to trigger UI updates e.g. from the LiveDataManager.
     */
    void dataChangedForReservation(const QString &resId);

    // for unit testing
    void setCurrentDateTime(const QDateTime &dt);
    QDateTime now() const;
    QDate today() const;

Q_SIGNALS:
    void todayRowChanged();

private:
    struct Element {
        explicit Element(ElementType type, const QDateTime &dateTime, const QVariant &data = {});
        explicit Element(const QString &resId, const QVariant &res, RangeType rt);

        QString batchId; // reservation batch id
        QVariant content; // non-reservation content
        QDateTime dt; // relevant date/time
        ElementType elementType;
        RangeType rangeType = SelfContained;
    };

    static bool elementLessThan(const Element &lhs, const Element &rhs);

    void batchAdded(const QString &resId);
    void insertElement(Element &&elem);
    void batchChanged(const QString &resId);
    void updateElement(const QString &resId, const QVariant &res, RangeType rangeType);
    void batchRenamed(const QString &oldBatchId, const QString &newBatchId);
    void batchRemoved(const QString &resId);

    void tripGroupAdded(const QString &groupId);
    void tripGroupChanged(const QString &groupId);
    void tripGroupRemoved(const QString &groupId);

    void dayChanged();
    void updateTodayMarker();
    void updateInformationElements();
    std::vector<Element>::iterator erasePreviousCountyInfo(std::vector<Element>::iterator it);
    void updateWeatherElements();

    ReservationManager *m_resMgr = nullptr;
    WeatherForecastManager *m_weatherMgr = nullptr;
    TripGroupManager *m_tripGroupManager = nullptr;
    std::vector<Element> m_elements;
    QString m_homeCountry;
    QDateTime m_unitTestTime;
    QTimer m_dayUpdateTimer;
};

#endif // TIMELINEMODEL_H
