# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"kde-itinerary\")

add_subdirectory(solidextras)
add_subdirectory(weather)
add_subdirectory(kandroidextras)
add_subdirectory(gpx)
add_subdirectory(app)

if (TARGET KF5::KIOWidgets)
    add_subdirectory(thumbnailer)
endif()
