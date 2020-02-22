/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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
