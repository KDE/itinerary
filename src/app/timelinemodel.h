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

#ifndef TIMELINEMODEL_H
#define TIMELINEMODEL_H

#include <QAbstractListModel>
#include <QDateTime>

class ReservationManager;
class WeatherForecastManager;

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
        ReservationIdsRole,
        ElementTypeRole,
        TodayEmptyRole,
        IsTodayRole,
        ElementRangeRole,
        CountryInformationRole,
        WeatherForecastRole
    };

    enum ElementType {
        Undefined,
        Flight,
        TrainTrip,
        BusTrip,
        Hotel,
        Restaurant,
        TouristAttraction,
        Event,
        TodayMarker,
        CountryInfo,
        WeatherForecast
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
    ~TimelineModel();

    void setReservationManager(ReservationManager *mgr);
    void setWeatherForecastManager(WeatherForecastManager *mgr);
    void setHomeCountryIsoCode(const QString &isoCode);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    int todayRow() const;

signals:
    void todayRowChanged();

private:
    struct Element {
        explicit Element(ElementType type, const QDateTime &dateTime, const QVariant &data = {});
        explicit Element(const QString &resId, const QVariant &res, RangeType rt);

        QStringList ids; // reservation ids (multiple entries in case of mult-traveller merging), QStringList as we need QML compatibility...
        QVariant content; // non-reservation content
        QDateTime dt; // relevant date/time
        ElementType elementType;
        RangeType rangeType = SelfContained;
    };

    void reservationAdded(const QString &resId);
    void insertElement(Element &&elem);
    void reservationUpdated(const QString &resId);
    void updateElement(const QString &resId, const QVariant &res, RangeType rangeType);
    void reservationRemoved(const QString &resId);

    void updateInformationElements();
    std::vector<Element>::iterator erasePreviousCountyInfo(std::vector<Element>::iterator it);
    void updateWeatherElements();

    ReservationManager *m_resMgr = nullptr;
    WeatherForecastManager *m_weatherMgr = nullptr;
    std::vector<Element> m_elements;
    QString m_homeCountry;
};

#endif // TIMELINEMODEL_H
