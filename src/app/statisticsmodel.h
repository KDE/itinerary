/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef STATISTICSMODEL_H
#define STATISTICSMODEL_H

#include <QDate>
#include <QObject>

#include <set>

class ReservationManager;
class TripGroupManager;
class TransferManager;

namespace KPublicTransport { class Journey; }

/** Statistics data item. */
class StatisticsItem
{
    Q_GADGET
    Q_PROPERTY(bool hasData MEMBER m_hasData CONSTANT)
    Q_PROPERTY(QString label MEMBER m_label CONSTANT)
    Q_PROPERTY(QString value MEMBER m_value CONSTANT)
    Q_PROPERTY(Trend trend MEMBER m_trend CONSTANT)

public:
    enum Trend { TrendUnknown, TrendUp, TrendDown, TrendUnchanged };
    Q_ENUM(Trend)

    StatisticsItem();
    explicit StatisticsItem(const QString &label, const QString &value, StatisticsItem::Trend trend = TrendUnknown, bool hasData = true);
    ~StatisticsItem();

    QString m_label;
    QString m_value;
    Trend m_trend = TrendUnknown;
    bool m_hasData = true;
};

Q_DECLARE_METATYPE(StatisticsItem)

/** Provides the data shown in the statistics page. */
class StatisticsModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(StatisticsItem totalCount READ totalCount NOTIFY changed)
    Q_PROPERTY(StatisticsItem totalDistance READ totalDistance NOTIFY changed)
    Q_PROPERTY(StatisticsItem totalNights READ totalNights NOTIFY changed)
    Q_PROPERTY(StatisticsItem totalCO2 READ totalCO2 NOTIFY changed)
    Q_PROPERTY(StatisticsItem visitedCountries READ visitedCountries NOTIFY changed)

    Q_PROPERTY(StatisticsItem flightCount READ flightCount NOTIFY changed)
    Q_PROPERTY(StatisticsItem flightDistance READ flightDistance NOTIFY changed)
    Q_PROPERTY(StatisticsItem flightCO2 READ flightCO2 NOTIFY changed)

    Q_PROPERTY(StatisticsItem trainCount READ trainCount NOTIFY changed)
    Q_PROPERTY(StatisticsItem trainDistance READ trainDistance NOTIFY changed)
    Q_PROPERTY(StatisticsItem trainCO2 READ trainCO2 NOTIFY changed)

    Q_PROPERTY(StatisticsItem busCount READ busCount NOTIFY changed)
    Q_PROPERTY(StatisticsItem busDistance READ busDistance NOTIFY changed)
    Q_PROPERTY(StatisticsItem busCO2 READ busCO2 NOTIFY changed)

    Q_PROPERTY(StatisticsItem carCount READ carCount NOTIFY changed)
    Q_PROPERTY(StatisticsItem carDistance READ carDistance NOTIFY changed)
    Q_PROPERTY(StatisticsItem carCO2 READ carCO2 NOTIFY changed)

    Q_PROPERTY(StatisticsItem boatCount READ boatCount NOTIFY changed)
    Q_PROPERTY(StatisticsItem boatDistance READ boatDistance NOTIFY changed)
    Q_PROPERTY(StatisticsItem boatCO2 READ boatCO2 NOTIFY changed)

    Q_PROPERTY(ReservationManager *reservationManager MEMBER m_resMgr WRITE setReservationManager NOTIFY setupChanged)
    Q_PROPERTY(TripGroupManager *tripGroupManager MEMBER m_tripGroupMgr WRITE setTripGroupManager NOTIFY setupChanged)
    Q_PROPERTY(TransferManager *transferManager MEMBER m_transferMgr WRITE setTransferManager NOTIFY setupChanged)

public:
    explicit StatisticsModel(QObject *parent = nullptr);
    ~StatisticsModel() override;

    void setReservationManager(ReservationManager *resMgr);
    void setTripGroupManager(TripGroupManager *tripGroupMgr);
    void setTransferManager(TransferManager *transferMgr);

    Q_INVOKABLE void setTimeRange(const QDate &begin, const QDate &end);

    [[nodiscard]] StatisticsItem totalCount() const;
    [[nodiscard]] StatisticsItem totalDistance() const;
    [[nodiscard]] StatisticsItem totalNights() const;
    [[nodiscard]] StatisticsItem totalCO2() const;
    [[nodiscard]] StatisticsItem visitedCountries() const;

    [[nodiscard]] StatisticsItem flightCount() const;
    [[nodiscard]] StatisticsItem flightDistance() const;
    [[nodiscard]] StatisticsItem flightCO2() const;

    [[nodiscard]] StatisticsItem trainCount() const;
    [[nodiscard]] StatisticsItem trainDistance() const;
    [[nodiscard]] StatisticsItem trainCO2() const;

    [[nodiscard]] StatisticsItem busCount() const;
    [[nodiscard]] StatisticsItem busDistance() const;
    [[nodiscard]] StatisticsItem busCO2() const;

    [[nodiscard]] StatisticsItem carCount() const;
    [[nodiscard]] StatisticsItem carDistance() const;
    [[nodiscard]] StatisticsItem carCO2() const;

    [[nodiscard]] StatisticsItem boatCount() const;
    [[nodiscard]] StatisticsItem boatDistance() const;
    [[nodiscard]] StatisticsItem boatCO2() const;

    /** Computes the estimated CO2 emission for @p res. */
    [[nodiscard]] static double co2Emission(const QVariant &res);

Q_SIGNALS:
    void setupChanged();
    void changed();

private:
    void recompute();
    [[nodiscard]] bool isRelevantTripGroup(const QString &tgId) const;

    ReservationManager *m_resMgr = nullptr;
    TripGroupManager *m_tripGroupMgr = nullptr;
    TransferManager *m_transferMgr = nullptr;
    QDate m_begin;
    QDate m_end;

    enum AggregateType { Total, Flight, Train, Bus, Car, Boat, AGGREGATE_TYPE_COUNT };
    enum StatType { TripCount, Distance, CO2, STAT_TYPE_COUNT };

    [[nodiscard]] static AggregateType typeForReservation(const QVariant &res);
    [[nodiscard]] static double co2emission(AggregateType type, double distance);
    void computeStats(const QString &resId, const QVariant &res, int (&statData)[AGGREGATE_TYPE_COUNT][STAT_TYPE_COUNT]);
    void computeStats(const KPublicTransport::Journey &journey, int (&statData)[AGGREGATE_TYPE_COUNT][STAT_TYPE_COUNT]);

    [[nodiscard]] StatisticsItem::Trend trend(int current, int prev) const;
    [[nodiscard]] StatisticsItem::Trend trend(AggregateType type, StatType stat) const;

    std::set<QString> m_countries;

    int m_statData[AGGREGATE_TYPE_COUNT][STAT_TYPE_COUNT];
    int m_prevStatData[AGGREGATE_TYPE_COUNT][STAT_TYPE_COUNT];
    bool m_hasData[AGGREGATE_TYPE_COUNT];
    int m_hotelCount = 0;
    int m_prevHotelCount = 0;
    int m_tripGroupCount = 0;
    int m_prevTripGroupCount = 0;
};

#endif // STATISTICSMODEL_H
