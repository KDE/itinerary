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

#include "timelineelement.h"
#include "transfer.h"

#include <QAbstractListModel>
#include <QDateTime>
#include <QTimer>

class ReservationManager;
class WeatherForecastManager;
class TripGroupManager;
class TransferManager;

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
        TripGroupRole,
        TransferRole,
    };

    explicit TimelineModel(QObject *parent = nullptr);
    ~TimelineModel() override;

    void setReservationManager(ReservationManager *mgr);
    void setWeatherForecastManager(WeatherForecastManager *mgr);
    TripGroupManager* tripGroupManager() const;
    void setTripGroupManager(TripGroupManager *mgr);
    void setHomeCountryIsoCode(const QString &isoCode);
    void setTransferManager(TransferManager *mgr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    int todayRow() const;

    // for unit testing
    void setCurrentDateTime(const QDateTime &dt);
    QDateTime now() const;
    QDate today() const;

Q_SIGNALS:
    void todayRowChanged();

private:
    void batchAdded(const QString &resId);
    void insertElement(TimelineElement &&elem);
    void batchChanged(const QString &resId);
    void updateElement(const QString &resId, const QVariant &res, TimelineElement::RangeType rangeType);
    void batchRenamed(const QString &oldBatchId, const QString &newBatchId);
    void batchRemoved(const QString &resId);

    void tripGroupAdded(const QString &groupId);
    void tripGroupChanged(const QString &groupId);
    void tripGroupRemoved(const QString &groupId);

    void transferChanged(const Transfer &transfer);
    void transferRemoved(const QString &resId, Transfer::Alignment alignment);

    void dayChanged();
    void updateTodayMarker();
    void updateInformationElements();
    std::vector<TimelineElement>::iterator erasePreviousCountyInfo(std::vector<TimelineElement>::iterator it);
    void updateWeatherElements();
    void updateTransfersForBatch(const QString &batchId);

    ReservationManager *m_resMgr = nullptr;
    WeatherForecastManager *m_weatherMgr = nullptr;
    TripGroupManager *m_tripGroupManager = nullptr;
    TransferManager *m_transferManager = nullptr;
    std::vector<TimelineElement> m_elements;
    QString m_homeCountry;
    QDateTime m_unitTestTime;
    QTimer m_dayUpdateTimer;
};

#endif // TIMELINEMODEL_H
