/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WEATHERTILE_H
#define WEATHERTILE_H

#include <QMetaType>

#include <cmath>
#include <cstdint>
#include <functional>

/** Weather forecast data tile coordinates */
struct WeatherTile {
    // tile size in 1/nth of a degree
    static const constexpr auto Size = 10.0;

    inline constexpr WeatherTile() = default;
    inline WeatherTile(float latitude, float longitude)
        : lat(int16_t(::round(latitude * WeatherTile::Size)))
        , lon(int16_t(::round(longitude * WeatherTile::Size)))
    {
    }

    inline bool operator<(WeatherTile other) const
    {
        return std::tie(lat, lon) < std::tie(other.lat, other.lon);
    }

    inline constexpr bool operator==(WeatherTile other) const
    {
        return lat == other.lat && lon == other.lon;
    }

    inline constexpr float latitude() const
    {
        return lat / Size;
    }

    inline constexpr float longitude() const
    {
        return lon / Size;
    }

    int16_t lat = 0;
    int16_t lon = 0;
};

Q_DECLARE_METATYPE(WeatherTile)

namespace std
{
template<>
struct hash<WeatherTile> {
    inline std::size_t operator()(const WeatherTile &t) const noexcept
    {
        return std::hash<uint32_t>{}(t.lat << 16 | t.lon);
    }
};
}

#endif // WEATHERTILE_H
