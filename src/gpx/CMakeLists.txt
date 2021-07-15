# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(GpxIo STATIC
    gpxreader.cpp
    gpxwriter.cpp
)
target_include_directories(GpxIo PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>")
target_link_libraries(GpxIo PUBLIC Qt::Core)
