/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "weatherforecast.h"
#include "weathertile.h"

#include <KHolidays/SunRiseSet>

#include <QDateTime>
#include <QDebug>

#include <limits>

class WeatherForecastPrivate : public QSharedData {
public:
    bool useDayTimeIcon() const;

    QDateTime m_dt;
    WeatherTile m_tile;
    float m_minTemp = std::numeric_limits<float>::max();
    float m_maxTemp = std::numeric_limits<float>::lowest();
    float m_precipitation = std::numeric_limits<float>::lowest();
    float m_windSpeed = std::numeric_limits<float>::lowest();
    WeatherForecast::SymbolType m_symbol = WeatherForecast::None;
    int m_range = 0;
};

WeatherForecast::WeatherForecast()
    : d(new WeatherForecastPrivate)
{
}

WeatherForecast::WeatherForecast(const WeatherForecast&) = default;
WeatherForecast::~WeatherForecast() = default;
WeatherForecast& WeatherForecast::operator=(const WeatherForecast&) = default;

bool WeatherForecast::isValid() const
{
    return d->m_dt.isValid();
}

QDateTime WeatherForecast::dateTime() const
{
    return d->m_dt;
}

void WeatherForecast::setDateTime(const QDateTime &dt)
{
    d.detach();
    d->m_dt = dt;
}

float WeatherForecast::minimumTemperature() const
{
    return d->m_minTemp;
}

void WeatherForecast::setMinimumTemperature(float t)
{
    d.detach();
    d->m_minTemp = t;
}

float WeatherForecast::maximumTemperature() const
{
    return d->m_maxTemp;
}

void WeatherForecast::setMaximumTemperature(float t)
{
    d.detach();
    d->m_maxTemp = t;
}

WeatherForecast::SymbolType WeatherForecast::symbolType() const
{
    return d->m_symbol | (d->m_windSpeed > 10.8f ? WeatherForecast::Wind : WeatherForecast::None);
}

void WeatherForecast::setSymbolType(WeatherForecast::SymbolType type)
{
    d.detach();
    d->m_symbol = type;
}

// breeze icon mapping
struct icon_map_t {
    WeatherForecast::SymbolType mask;
    const char *dayIcon;
    const char *nightIcon;
};

static const icon_map_t icon_map[] = {
    { WeatherForecast::Snow, "weather-snow", "weather-snow" },
    { WeatherForecast::LightSnow | WeatherForecast::Clear, "weather-snow-scattered-day", "weather-snow-scattered-night" },
    { WeatherForecast::LightSnow, "weather-snow-scattered", "weather-snow-scattered" },

    { WeatherForecast::Hail, "weather-hail", "weather-hail" },

    { WeatherForecast::Clear | WeatherForecast::ThunderStorm, "weather-storm-day", "weather-storm-night" },
    { WeatherForecast::ThunderStorm, "weather-storm", "weather-storm" },

    { WeatherForecast::Clear | WeatherForecast::Rain, "weather-showers-day", "weather-showers-night" },
    { WeatherForecast::Rain, "weather-showers", "weather-showers" },
    { WeatherForecast::Clear | WeatherForecast::LightRain, "weather-showers-scattered-day", "weather-showers-scattered-night" },
    { WeatherForecast::LightRain, "weather-showers-scattered", "weather-showers-scattered" },

    { WeatherForecast::Clear | WeatherForecast::Clouds | WeatherForecast::Wind, "weather-clouds-wind", "weather-clouds-wind-night" },
    { WeatherForecast::Clear | WeatherForecast::Clouds, "weather-clouds", "weather-clouds-night" },
    { WeatherForecast::Clouds | WeatherForecast::Wind, "weather-many-clouds-wind", "weather-many-clouds-wind" },
    { WeatherForecast::Clouds, "weather-many-clouds", "weather-many-clouds" },
    { WeatherForecast::Fog, "weather-fog", "weather-fog" },
    { WeatherForecast::LightClouds | WeatherForecast::Wind, "weather-few-clouds-wind", "weather-few-clouds-wind-night" },
    { WeatherForecast::LightClouds, "weather-few-clouds", "weather-few-clouds-night" },
    { WeatherForecast::Clear | WeatherForecast::Wind, "weather-clear-wind", "weather-clear-wind-night" },
    { WeatherForecast::Clear, "weather-clear", "weather-clear-night" }
};

bool WeatherForecastPrivate::useDayTimeIcon() const
{
    if (m_range >= 24) {
        return true;
    }

    const auto endDt = m_dt.addSecs(m_range * 3600);
    const auto sunrise = QDateTime(m_dt.date(), KHolidays::SunRiseSet::utcSunrise(m_dt.date(), m_tile.latitude(), m_tile.longitude()), Qt::UTC);
    const auto sunset = QDateTime(m_dt.date(), KHolidays::SunRiseSet::utcSunset(m_dt.date(), m_tile.latitude(), m_tile.longitude()), Qt::UTC);
    // check overlap for two days, otherwise we might miss one on a day boundary
    return (sunrise < endDt && sunset > m_dt) || (sunrise.addDays(1) < endDt && sunset.addDays(1) > m_dt);
}

QString WeatherForecast::symbolIconName() const
{
    for (const auto &icon : icon_map) {
        if ((icon.mask & symbolType()) == icon.mask) {
            return QLatin1String(d->useDayTimeIcon() ? icon.dayIcon : icon.nightIcon);
        }
    }
    return {};
}

float WeatherForecast::precipitation() const
{
    return std::max(d->m_precipitation, 0.0f);
}

void WeatherForecast::setPrecipitation(float precipitation)
{
    d.detach();
    d->m_precipitation = precipitation;
}

float WeatherForecast::windSpeed() const
{
    return std::max(d->m_windSpeed, 0.0f);
}

void WeatherForecast::setWindSpeed(float speed)
{
    d.detach();
    d->m_windSpeed = speed;
}

void WeatherForecast::merge(const WeatherForecast &other)
{
    d.detach();
    if (d->m_minTemp == std::numeric_limits<float>::max() || other.range() <= d->m_range) {
        d->m_minTemp = std::min(other.minimumTemperature(), d->m_minTemp);
    }
    if (d->m_maxTemp == std::numeric_limits<float>::min() || other.range() <= d->m_range) {
        d->m_maxTemp = std::max(other.maximumTemperature(), d->m_maxTemp);
    }
    if (d->m_precipitation < 0.0f || other.range() <= d->m_range) {
        d->m_precipitation = std::max(other.precipitation(), d->m_precipitation);
    }
    if (d->m_windSpeed < 0.0f || other.range() <= d->m_range) {
        d->m_windSpeed = std::max(other.windSpeed(), d->m_windSpeed);
    }

    if (d->m_symbol == None || other.range() <= d->m_range) {
        d->m_symbol |= other.symbolType();
    }
}

int WeatherForecast::range() const
{
    return d->m_range;
}

void WeatherForecast::setRange(int hours)
{
    d.detach();
    d->m_range = hours;
}

WeatherTile WeatherForecast::tile() const
{
    return d->m_tile;
}

void WeatherForecast::setTile(WeatherTile tile)
{
    d.detach();
    d->m_tile = tile;
}

bool WeatherForecast::isSevere() const
{
    // heat: https://en.wikipedia.org/wiki/Heat_index
    // TODO to do this properly we need the humidity value as well
    if (d->m_maxTemp > 35.0) {
        return true;
    }

    // cold
    if (d->m_minTemp < -20.0) {
        return true;
    }

    // precipitation: https://de.wikipedia.org/wiki/Unwetter
    if ((d->m_precipitation / d->m_range) > 25.0 || (d->m_precipitation > 35.0 && d->m_range >= 6)) {
        return true;
    }

    // wind: https://en.wikipedia.org/wiki/Beaufort_scale
    if (d->m_windSpeed > 17.5) {
        return true;
    }


    return false;
}
