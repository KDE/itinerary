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

#include "weatherforecast.h"

#include <QDateTime>

#include <limits>

class WeatherForecastPrivate : public QSharedData {
public:
    QDateTime m_dt;
    float m_minTemp = std::numeric_limits<float>::max();
    float m_maxTemp = std::numeric_limits<float>::min();
    WeatherForecast::SymbolType m_symbol = WeatherForecast::Unknown;
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
    return d->m_symbol;
}

void WeatherForecast::setSymbolType(WeatherForecast::SymbolType type)
{
    d.detach();
    d->m_symbol = type;
}

QString WeatherForecast::symbolIconName() const
{
    // TODO night icon handling
    switch (symbolType()) {
        case Unknown: return {};
        case Clear: return QStringLiteral("weather-clear");
        case LightClouds: return QStringLiteral("weather-few-clouds");
        case PartlyCloudy: return QStringLiteral("weather-clouds");
        case RainShowers: return QStringLiteral("weather-showers-day");
        case LightRainShowers: return QStringLiteral("weather-showers-scattered-day");
        case LightSnowShowers: return QStringLiteral("weather-snow-scattered-day");
        case ThunderStormShowers: return QStringLiteral("weather-storm-day");
        // ^ have day and night variants
        // v only universal variants
        case Clouds: return QStringLiteral("weather-many-clouds");
        case Fog: return QStringLiteral("weather-fog");
        case Rain: return QStringLiteral("weather-showers");
        case LightRain: return QStringLiteral("weather-showers-scattered");
        case Hail: return QStringLiteral("weather-hail");
        case Snow: return QStringLiteral("weather-snow");
        case LightSnow: return QStringLiteral("weather-snow-scattered");
        case ThunderStorm: return QStringLiteral("weather-storm");
    }
    return {};
}

void WeatherForecast::merge(WeatherForecast &other)
{
    d.detach();
    d->m_minTemp = std::min(other.minimumTemperature(), d->m_minTemp);
    d->m_maxTemp = std::max(other.maximumTemperature(), d->m_maxTemp);

    if (d->m_symbol == Unknown) {
        d->m_symbol = other.symbolType();
    }
}
