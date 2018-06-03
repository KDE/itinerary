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
#include <functional>

/** Weather forecast data tile coordinates */
struct WeatherTile
{
    // tile size in 1/nth of a degree
    static const constexpr auto Size = 10.0;

    inline constexpr WeatherTile() = default;
    inline WeatherTile(float latitude, float longitude)
        : lat(uint16_t(::round(latitude * WeatherTile::Size)))
        , lon(uint16_t(::round(longitude * WeatherTile::Size)))
    {}

    inline bool operator<(WeatherTile other) const
    {
        return std::tie(lat, lon) < std::tie(other.lat, other.lon);
    }

    inline constexpr bool operator==(WeatherTile other) const
    {
        return lat == other.lat && lon == other.lon;
    }

    uint16_t lat = 0;
    uint16_t lon = 0;
};

Q_DECLARE_METATYPE(WeatherTile)

namespace std
{
    template<> struct hash<WeatherTile>
    {
        inline std::size_t operator()(const WeatherTile &t) const noexcept
        {
            return std::hash<uint32_t>{}(t.lat << 16 | t.lon);
        }
    };
}

#endif // WEATHERTILE_H

