# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"kde-itinerary\")

add_subdirectory(solidextras)
add_subdirectory(weather)
add_subdirectory(kjniextras)
add_subdirectory(kandroidextras)
add_subdirectory(gpx)
if (KF6CalendarCore_VERSION VERSION_LESS 6.28)
    add_subdirectory(calendarextras)
endif()
add_subdirectory(app)
add_subdirectory(matrix)

if (TARGET KF6::FileMetaData)
    add_subdirectory(metadataextractor)
endif()
if (TARGET KF6::KIOWidgets)
    add_subdirectory(thumbnailer)
endif()
