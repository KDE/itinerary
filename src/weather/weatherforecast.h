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

#ifndef WEATHERFORECAST_H
#define WEATHERFORECAST_H

#include <QExplicitlySharedDataPointer>
#include <QMetaType>

#include <cstdint>

class QDateTime;

class WeatherForecastPrivate;

/** Weather forecast data */
class WeatherForecast
{
    Q_GADGET
    Q_PROPERTY(bool valid READ isValid CONSTANT)
    Q_PROPERTY(float temperature READ temperature CONSTANT)
    Q_PROPERTY(QString symbolIconName READ symbolIconName CONSTANT)

public:
    /** Weather symbol, aligned with the Breeze icon theme, not the data source symbols. */
    enum SymbolType : uint8_t {
        Unknown,

        Clear,
        LightClouds,
        PartlyCloudy,
        RainShowers,
        LightRainShowers,
        LightSnowShowers,
        ThunderStormShowers,
        // ^ have day and night variants
        // v only universal variants
        Clouds,
        Fog,
        Rain,
        LightRain,
        Hail,
        Snow,
        LightSnow,
        ThunderStorm
    };
    Q_ENUM(SymbolType)

    WeatherForecast();
    WeatherForecast(const WeatherForecast&);
    ~WeatherForecast();
    WeatherForecast& operator=(const WeatherForecast&);

    bool isValid() const;

    /** The time this data is valid for. */
    QDateTime dateTime() const;
    void setDateTime(const QDateTime &dt);

    // TODO is this enough or do we need min/max ranges?
    float temperature() const;
    void setTemperature(float t);

    SymbolType symbolType() const;
    void setSymbolType(SymbolType type);
    QString symbolIconName() const;

    // TODO precipitation probability

    /** Merge with @p other. */
    void merge(WeatherForecast &other);

private:
    QExplicitlySharedDataPointer<WeatherForecastPrivate> d;
};

Q_DECLARE_METATYPE(WeatherForecast)

#endif // WEATHERFORECAST_H
