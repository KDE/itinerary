// SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "timelineelement.h"
#include "abstracttimelinemodel.h"
#include "transfer.h"
#include "tripgroup.h"
#include "tripgroupmanager.h"
#include "reservationmanager.h"
#include "transfermanager.h"

#include <QAbstractListModel>
#include <QDateTime>
#include <QTimer>

class TripTimelineModel : public AbstractTimelineModel
{
    Q_OBJECT
    Q_PROPERTY(int todayRow READ todayRow NOTIFY todayRowChanged)
    Q_PROPERTY(QString currentBatchId READ currentBatchId NOTIFY currentBatchChanged)

    /// This property holds the trip group manager.
    Q_PROPERTY(TripGroupManager *tripGroupManager READ tripGroupManager WRITE setTripGroupManager NOTIFY tripGroupManagerChanged)

    /// This property holds the reservation manager.
    Q_PROPERTY(ReservationManager *reservationManager READ reservationManager WRITE setReservationManager NOTIFY reservationManagerChanged)

    /// This property holds the transfer manager.
    Q_PROPERTY(TransferManager *transferManager READ transferManager WRITE setTransferManager NOTIFY transferManagerChanged)

    /// This property holds the trip group which should be displayed by this timeline model.
    Q_PROPERTY(TripGroup tripGroup READ tripGroup WRITE setTripGroup NOTIFY tripGroupChanged)

public:
    enum Role {
        SectionHeaderRole = Qt::UserRole + 1,
        BatchIdRole,
        ElementTypeRole,
        TodayEmptyRole,
        IsTodayRole,
        LocationInformationRole,
        ReservationsRole, // for unit testing
        TransferRole,
        StartDateTimeRole,
        EndDateTimeRole,
        IsTimeboxedRole,
        IsCanceledRole,
    };
    Q_ENUM(Role)

    explicit TripTimelineModel(QObject *parent = nullptr);
    ~TripTimelineModel() override;

    [[nodiscard]]
    TripGroupManager *tripGroupManager() const;
    void setTripGroupManager(TripGroupManager *tripGroupManager);

    [[nodiscard]]
    ReservationManager *reservationManager() const override;
    void setReservationManager(ReservationManager *mgr);

    [[nodiscard]]
    TripGroup tripGroup() const;
    void setTripGroup(const TripGroup &tripGroup);

    [[nodiscard]]
    TransferManager *transferManager() const;
    void setTransferManager(TransferManager *mgr);

    void setHomeCountryIsoCode(const QString &isoCode);

    [[nodiscard]]
    QVariant data(const QModelIndex& index, int role) const override;

    [[nodiscard]]
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    [[nodiscard]]
    QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]]
    int todayRow() const;

    /** The most "current" batch to show with the "ticket check" action. */
    [[nodiscard]]
    QString currentBatchId() const;

    /** The location we are in at the given date/time. */
    [[nodiscard]]
    Q_INVOKABLE QVariant locationAtTime(const QDateTime &dt) const;

    // for unit testing
    void setCurrentDateTime(const QDateTime &dt);

    [[nodiscard]]
    QDateTime now() const;

    [[nodiscard]]
    QDate today() const;

Q_SIGNALS:
    void todayRowChanged();
    void currentBatchChanged();
    void reservationManagerChanged();
    void tripGroupManagerChanged();
    void tripGroupChanged();
    void transferManagerChanged();

private:
    void batchAdded(const QString &resId);
    void insertElement(TimelineElement &&elem);
    std::vector<TimelineElement>::iterator insertOrUpdate(std::vector<TimelineElement>::iterator it, TimelineElement &&elem);
    void batchChanged(const QString &resId);
    void updateElement(const QString &resId, const QVariant &res, TimelineElement::RangeType rangeType);
    void batchRenamed(const QString &oldBatchId, const QString &newBatchId);
    void batchRemoved(const QString &resId);

    void transferChanged(const Transfer &transfer);
    void transferRemoved(const QString &resId, Transfer::Alignment alignment);

    void dayChanged();
    void updateTodayMarker();
    void updateInformationElements();
    void updateTransfersForBatch(const QString &batchId);
    void updateElements();

    void scheduleCurrentBatchTimer();

    [[nodiscard]]
    bool isDateEmpty(const QDate &date) const;

    friend class TimelineElement;
    ReservationManager *m_resMgr = nullptr;
    TransferManager *m_transferManager = nullptr;
    TripGroupManager *m_tripGroupManager = nullptr;
    TripGroup m_tripGroup;
    std::vector<TimelineElement> m_elements;
    QString m_homeCountry;
    QDateTime m_unitTestTime;
    QTimer m_dayUpdateTimer;
    QTimer m_currentBatchTimer;
    bool m_todayEmpty = true;
};
