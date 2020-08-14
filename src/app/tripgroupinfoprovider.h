/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TRIPGROUPINFOPROVIDER_H
#define TRIPGROUPINFOPROVIDER_H

#include <QMetaType>

class ReservationManager;
class TripGroup;
class WeatherForecast;
class WeatherForecastManager;

/** Provides the information shown in the trip group delegate. */
class TripGroupInfoProvider
{
    Q_GADGET
public:
    TripGroupInfoProvider();
    ~TripGroupInfoProvider();

    void setReservationManager(ReservationManager *resMgr);
    void setWeatherForecastManager(WeatherForecastManager *mgr);

    Q_INVOKABLE WeatherForecast weatherForecast(const TripGroup &group) const;
    Q_INVOKABLE QVariantList locationInformation(const TripGroup &group, const QString &homeCountryIsoCode) const;

private:
    ReservationManager *m_resMgr = nullptr;
    WeatherForecastManager *m_weatherMgr = nullptr;
};

Q_DECLARE_METATYPE(TripGroupInfoProvider)

#endif // TRIPGROUPINFOPROVIDER_H
