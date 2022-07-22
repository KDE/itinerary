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

/** Statistics data item. */
class StatisticsItem
{
    Q_GADGET
    Q_PROPERTY(bool hasData MEMBER m_hasData CONSTANT)
    Q_PROPERTY(QString label MEMBER m_label CONSTANT)
    Q_PROPERTY(QString value MEMBER m_value CONSTANT)
    Q_PROPERTY(Trend trend MEMBER m_trend CONSTANT)

public:
    enum Trend {
        TrendUnknown,
        TrendUp,
        TrendDown,
        TrendUnchanged
    };
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

    Q_PROPERTY(ReservationManager* reservationManager READ reservationManager WRITE setReservationManager NOTIFY setupChanged)
    Q_PROPERTY(TripGroupManager* tripGroupManager READ tripGroupManager WRITE setTripGroupManager NOTIFY setupChanged)

public:
    explicit StatisticsModel(QObject *parent = nullptr);
    ~StatisticsModel() override;

    ReservationManager* reservationManager() const;
    void setReservationManager(ReservationManager *resMgr);
    TripGroupManager* tripGroupManager() const;
    void setTripGroupManager(TripGroupManager *tripGroupMgr);

    Q_INVOKABLE void setTimeRange(const QDate &begin, const QDate &end);

    StatisticsItem totalCount() const;
    StatisticsItem totalDistance() const;
    StatisticsItem totalNights() const;
    StatisticsItem totalCO2() const;
    StatisticsItem visitedCountries() const;

    StatisticsItem flightCount() const;
    StatisticsItem flightDistance() const;
    StatisticsItem flightCO2() const;

    StatisticsItem trainCount() const;
    StatisticsItem trainDistance() const;
    StatisticsItem trainCO2() const;

    StatisticsItem busCount() const;
    StatisticsItem busDistance() const;
    StatisticsItem busCO2() const;

    StatisticsItem carCount() const;
    StatisticsItem carDistance() const;
    StatisticsItem carCO2() const;

    StatisticsItem boatCount() const;
    StatisticsItem boatDistance() const;
    StatisticsItem boatCO2() const;

Q_SIGNALS:
    void setupChanged();
    void changed();

private:
    void recompute();

    ReservationManager *m_resMgr = nullptr;
    TripGroupManager *m_tripGroupMgr = nullptr;
    QDate m_begin;
    QDate m_end;

    enum AggregateType { Total, Flight, Train, Bus, Car, Boat, AGGREGATE_TYPE_COUNT };
    enum StatType { TripCount, Distance, CO2, STAT_TYPE_COUNT };

    AggregateType typeForReservation(const QVariant &res) const;
    int co2emission(AggregateType type, int distance) const;
    void computeStats(const QVariant &res, int (&statData)[AGGREGATE_TYPE_COUNT][STAT_TYPE_COUNT]);

    StatisticsItem::Trend trend(int current, int prev) const;
    StatisticsItem::Trend trend(AggregateType type, StatType stat) const;

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
