/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WEATHERFORECAST_H
#define WEATHERFORECAST_H

#include <QExplicitlySharedDataPointer>
#include <QMetaType>

#include <cstdint>

class QDateTime;

class WeatherForecastPrivate;
struct WeatherTile;

/** Weather forecast data */
class WeatherForecast
{
    Q_GADGET
    Q_PROPERTY(bool valid READ isValid CONSTANT)
    Q_PROPERTY(float minimumTemperature READ minimumTemperature CONSTANT)
    Q_PROPERTY(float maximumTemperature READ maximumTemperature CONSTANT)
    Q_PROPERTY(float precipitation READ precipitation CONSTANT)
    Q_PROPERTY(float windSpeed READ windSpeed CONSTANT)
    Q_PROPERTY(QString symbolIconName READ symbolIconName CONSTANT)
    Q_PROPERTY(int range READ range CONSTANT)
    Q_PROPERTY(bool isSevere READ isSevere STORED false)

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
        Wind = 1024
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

    /** Wind speed in m/s. */
    float windSpeed() const;
    void setWindSpeed(float speed);

    /** Weather symbol. */
    SymbolType symbolType() const;
    void setSymbolType(SymbolType type);
    QString symbolIconName() const;

    /** Merge with @p other. */
    void merge(const WeatherForecast &other);

    // internal for weighting different forecast elements
    int range() const;
    void setRange(int hours);

    /** Severe weather conditions. */
    bool isSevere() const;

    // internal for computing the day/night icons
    WeatherTile tile() const;
    void setTile(WeatherTile tile);

private:
    QExplicitlySharedDataPointer<WeatherForecastPrivate> d;
};

Q_DECLARE_METATYPE(WeatherForecast)
Q_DECLARE_OPERATORS_FOR_FLAGS(WeatherForecast::SymbolType)

#endif // WEATHERFORECAST_H
