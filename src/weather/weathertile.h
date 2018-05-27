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

#ifndef WEATHERTILE_H
#define WEATHERTILE_H

#include <QMetaType>

#include <cmath>
#include <cstdint>

/** Weather forecast data tile coordinates */
struct WeatherTile
{
    // tile size in 1/nth of a degree
    static const constexpr auto Size = 10.0;

    inline constexpr WeatherTile() = default;
    inline constexpr WeatherTile(float latitude, float longitude)
        : x(uint16_t(std::round(latitude * WeatherTile::Size)))
        , y(uint16_t(std::round(longitude * WeatherTile::Size)))
    {}

    inline constexpr bool operator<(WeatherTile other)
    {
        return std::tie(x, y) < std::tie(other.x, other.y);
    }

    inline constexpr bool operator==(WeatherTile other)
    {
        return x == other.x && y == other.y;
    }

    uint16_t x = 0;
    uint16_t y = 0;
};

Q_DECLARE_METATYPE(WeatherTile)

#endif // WEATHERTILE_H

