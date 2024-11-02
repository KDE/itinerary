/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TRIPGROUPCONTROLLER_H
#define TRIPGROUPCONTROLLER_H

#include "transfermanager.h"
#include "tripgroupmodel.h"
#include "weatherforecast.h"
#include "weatherforecastmanager.h"

#include <QObject>

/** Provides the information and actions shown in the trip group delegate. */
class TripGroupController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString tripGroupId MEMBER m_tgId NOTIFY tripGroupChanged)
    Q_PROPERTY(TripGroupModel *tripGroupModel MEMBER m_tripGroupModel WRITE setTripGroupModel NOTIFY setupChanged)
    Q_PROPERTY(WeatherForecastManager *weatherForecastManager MEMBER m_weatherMgr WRITE setWeatherForecastManager NOTIFY setupChanged)
    Q_PROPERTY(QString homeCountryIsoCode MEMBER m_homeCountry NOTIFY locationInfoChanged)
    Q_PROPERTY(QString homeCurrency MEMBER m_homeCurrency NOTIFY locationInfoChanged)

    Q_PROPERTY(WeatherForecast weatherForecast READ weatherForecast NOTIFY weatherForecastChanged)
    Q_PROPERTY(QVariantList locationInformation READ locationInformation NOTIFY locationInfoChanged)
    Q_PROPERTY(QStringList currencies READ currencies NOTIFY locationInfoChanged)

    Q_PROPERTY(TransferManager *transferManager MEMBER m_tranferMgr NOTIFY setupChanged)

    Q_PROPERTY(bool canMerge READ canMerge NOTIFY adjacencyChanged)
    Q_PROPERTY(bool canSplit READ canSplit NOTIFY tripGroupContentChanged)

    Q_PROPERTY(double totalDistance READ totalDistance NOTIFY tripGroupContentChanged)
    Q_PROPERTY(double totalCO2Emission READ totalCO2Emission NOTIFY tripGroupContentChanged)

public:
    explicit TripGroupController(QObject *parent = nullptr);
    ~TripGroupController();

    void setTripGroupModel(TripGroupModel *tgModel);
    void setWeatherForecastManager(WeatherForecastManager *weatherMgr);

    [[nodiscard]] WeatherForecast weatherForecast() const;
    [[nodiscard]] QVariantList locationInformation() const;
    [[nodiscard]] QStringList currencies() const;

    [[nodiscard]] bool canMerge() const;
    [[nodiscard]] bool canSplit() const;

    [[nodiscard]] double totalDistance() const;
    [[nodiscard]] double totalCO2Emission() const;

Q_SIGNALS:
    void setupChanged();
    void tripGroupChanged();
    void adjacencyChanged();
    void weatherForecastChanged();
    void locationInfoChanged();
    void tripGroupContentChanged();

private:
    TripGroupModel *m_tripGroupModel = nullptr;
    WeatherForecastManager *m_weatherMgr = nullptr;
    TransferManager *m_tranferMgr = nullptr;
    QString m_tgId;

    QString m_homeCountry;
    QString m_homeCurrency;
};

#endif // TRIPGROUPCONTROLLER_H
