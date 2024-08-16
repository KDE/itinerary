/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TIMELINEMODEL_H
#define TIMELINEMODEL_H

#include "timelineelement.h"
#include "transfer.h"
#include "tripgroup.h"

#include <QAbstractListModel>
#include <QDateTime>
#include <QTimer>

class ReservationManager;
class WeatherForecastManager;
class TripGroupManager;
class TransferManager;

class TimelineModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int todayRow READ todayRow NOTIFY todayRowChanged)

    Q_PROPERTY(ReservationManager* reservationManager MEMBER m_resMgr WRITE setReservationManager NOTIFY setupChanged)
    Q_PROPERTY(TransferManager* transferManager MEMBER m_transferManager WRITE setTransferManager NOTIFY setupChanged)
    Q_PROPERTY(TripGroupManager* tripGroupManager READ tripGroupManager WRITE setTripGroupManager NOTIFY setupChanged)
    Q_PROPERTY(WeatherForecastManager* weatherForecastManager MEMBER m_weatherMgr WRITE setWeatherForecastManager NOTIFY setupChanged)
    Q_PROPERTY(QString homeCountryIsoCode MEMBER m_homeCountry WRITE setHomeCountryIsoCode NOTIFY setupChanged)

    /** Show only a single trip group. */
    Q_PROPERTY(QString tripGroupId MEMBER m_tripGroupId WRITE setTripGroupId NOTIFY tripGroupIdChanged)
    /** Initial location before the begin of the timeline, if known.
     *  Used for weather and location information.
     */
    Q_PROPERTY(QVariant initialLocation MEMBER m_initialLocation NOTIFY initialLocationChanged)

public:
    enum Role {
        SectionHeaderRole = Qt::UserRole + 1,
        BatchIdRole,
        ElementTypeRole,
        TodayEmptyRole,
        IsTodayRole,
        ElementRangeRole,
        LocationInformationRole,
        WeatherForecastRole,
        ReservationsRole, // for unit testing
        TripGroupIdRole,
        TripGroupRole,
        TransferRole,
        StartDateTimeRole,
        EndDateTimeRole,
        IsTimeboxedRole,
        IsCanceledRole,
    };
    Q_ENUM(Role)

    explicit TimelineModel(QObject *parent = nullptr);
    ~TimelineModel() override;

    void setReservationManager(ReservationManager *mgr);
    void setWeatherForecastManager(WeatherForecastManager *mgr);
    [[nodiscard]] TripGroupManager* tripGroupManager() const;
    void setTripGroupManager(TripGroupManager *mgr);
    void setHomeCountryIsoCode(const QString &isoCode);
    void setTransferManager(TransferManager *mgr);
    void setTripGroupId(const QString &tgId);

    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int todayRow() const;

    // for unit testing
    void setCurrentDateTime(const QDateTime &dt);
    [[nodiscard]] QDateTime now() const;
    [[nodiscard]] QDate today() const;

Q_SIGNALS:
    void setupChanged();
    void todayRowChanged();
    void tripGroupIdChanged();
    void initialLocationChanged();

private:
    void populate();
    void populateReservation(const QString &resId);

    void batchAdded(const QString &resId);
    void insertElement(TimelineElement &&elem);
    std::vector<TimelineElement>::iterator insertOrUpdate(std::vector<TimelineElement>::iterator it, TimelineElement &&elem);
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
    void updateWeatherElements();
    void updateTransfersForBatch(const QString &batchId);

    [[nodiscard]] bool isDateEmpty(const QDate &date) const;

    friend class TimelineElement;
    ReservationManager *m_resMgr = nullptr;
    WeatherForecastManager *m_weatherMgr = nullptr;
    TripGroupManager *m_tripGroupManager = nullptr;
    TransferManager *m_transferManager = nullptr;
    std::vector<TimelineElement> m_elements;
    QString m_tripGroupId;
    TripGroup m_tripGroup;
    QString m_homeCountry;
    QVariant m_initialLocation;
    QDateTime m_unitTestTime;
    QTimer m_dayUpdateTimer;
    bool m_todayEmpty = true;
    bool m_isPopulated = false;
};

#endif // TIMELINEMODEL_H
