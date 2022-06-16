/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

class TimelineModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int todayRow READ todayRow NOTIFY todayRowChanged)
    Q_PROPERTY(QString currentBatchId READ currentBatchId NOTIFY currentBatchChanged)

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
    TripGroupManager* tripGroupManager() const;
    void setTripGroupManager(TripGroupManager *mgr);
    void setHomeCountryIsoCode(const QString &isoCode);
    void setTransferManager(TransferManager *mgr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    int todayRow() const;

    /** The most "current" batch to show with the "ticket check" action. */
    QString currentBatchId() const;

    /** The location we are in at the given date/time. */
    Q_INVOKABLE QVariant locationAtTime(const QDateTime &dt) const;

    // for unit testing
    void setCurrentDateTime(const QDateTime &dt);
    QDateTime now() const;
    QDate today() const;

Q_SIGNALS:
    void todayRowChanged();
    void currentBatchChanged();

private:
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

    void scheduleCurrentBatchTimer();
    bool isDateEmpty(const QDate &date) const;

    friend class TimelineElement;
    ReservationManager *m_resMgr = nullptr;
    WeatherForecastManager *m_weatherMgr = nullptr;
    TripGroupManager *m_tripGroupManager = nullptr;
    TransferManager *m_transferManager = nullptr;
    std::vector<TimelineElement> m_elements;
    QString m_homeCountry;
    QDateTime m_unitTestTime;
    QTimer m_dayUpdateTimer;
    QTimer m_currentBatchTimer;
    bool m_todayEmpty = true;
};

#endif // TIMELINEMODEL_H
