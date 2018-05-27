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

class WeatherForecastPrivate : public QSharedData {
public:
    QDateTime m_dt;
    float m_temp = -300;
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

float WeatherForecast::temperature() const
{
    return d->m_temp;
}

void WeatherForecast::setTemperature(float t)
{
    d.detach();
    d->m_temp = t;
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
    return {}; // TODO map to breeze icons and determine day/night version
}

void WeatherForecast::merge(WeatherForecast &other)
{
    d.detach();
    if (d->m_temp < -273.15) {
        d->m_temp = other.temperature();
    }
    if (d->m_symbol == Unknown) {
        d->m_symbol = other.symbolType();
    }
}
