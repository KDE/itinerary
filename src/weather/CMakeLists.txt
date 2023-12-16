# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(itinerary-weather STATIC)
target_sources(itinerary-weather PRIVATE
    weatherforecast.cpp
    weatherforecastmanager.cpp
)

target_link_libraries(itinerary-weather
    PUBLIC
        Qt::Core
    PRIVATE
        KF6::Holidays
        Qt::Network
        ZLIB::ZLIB
)
