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
class WeatherTile;

/** Weather forecast data */
class WeatherForecast
{
    Q_GADGET
    Q_PROPERTY(bool valid READ isValid CONSTANT)
    Q_PROPERTY(float minimumTemperature READ minimumTemperature CONSTANT)
    Q_PROPERTY(float maximumTemperature READ maximumTemperature CONSTANT)
    Q_PROPERTY(float precipitation READ precipitation CONSTANT)
    Q_PROPERTY(QString symbolIconName READ symbolIconName CONSTANT)

public:
    enum SymbolFlag : uint16_t {
        None = 0,
        Clear = 1,
        LightClouds = 2,
        Clouds = 4,
        LightRain = 8,
        Rain = 16,
        LightSnow = 32,
        Snow = 64,
        Hail = 128,
        ThunderStorm = 256,
        Fog = 512,
    };
    /** Weather symbol.
     *  Represented as flags so we can easily merge this for longer time periods.
     */
    Q_DECLARE_FLAGS(SymbolType, SymbolFlag)
    Q_FLAG(SymbolType)

    WeatherForecast();
    WeatherForecast(const WeatherForecast&);
    ~WeatherForecast();
    WeatherForecast& operator=(const WeatherForecast&);

    bool isValid() const;

    /** The time this data is valid for. */
    QDateTime dateTime() const;
    void setDateTime(const QDateTime &dt);

    /** Temperature range. */
    float minimumTemperature() const;
    void setMinimumTemperature(float t);
    float maximumTemperature() const;
    void setMaximumTemperature(float t);

    /** Precipitation in mm/m². */
    float precipitation() const;
    void setPrecipitation(float precipitation);

    /** Weather symbol. */
    SymbolType symbolType() const;
    void setSymbolType(SymbolType type);
    QString symbolIconName() const;

    /** Merge with @p other. */
    void merge(const WeatherForecast &other);

    // internal for weighting different forecast elements
    int range() const;
    void setRange(int hours);
    // internal for computing the day/night icons
    WeatherTile tile() const;
    void setTile(WeatherTile tile);

private:
    QExplicitlySharedDataPointer<WeatherForecastPrivate> d;
};

Q_DECLARE_METATYPE(WeatherForecast)
Q_DECLARE_OPERATORS_FOR_FLAGS(WeatherForecast::SymbolType)

#endif // WEATHERFORECAST_H
