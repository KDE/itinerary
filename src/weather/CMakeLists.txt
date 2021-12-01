# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-weather.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-weather.h)

add_library(itinerary-weather STATIC)
target_sources(itinerary-weather PRIVATE
    weatherforecast.cpp
    weatherforecastmanager.cpp
)

target_link_libraries(itinerary-weather
    PUBLIC
        Qt::Core
    PRIVATE
        Qt::Network
        ZLIB::ZLIB
)
if (TARGET KF5::Holidays)
    target_link_libraries(itinerary-weather PRIVATE KF5::Holidays)
endif()
