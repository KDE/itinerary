# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"kde-itinerary\")

add_subdirectory(solidextras)
add_subdirectory(weather)
if (ANDROID)
    add_subdirectory(kandroidextras)
endif()
add_subdirectory(app)
