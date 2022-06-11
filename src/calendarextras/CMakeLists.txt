# SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(KCalendarCoreExtras)
generate_export_header(KCalendarCoreExtras BASE_NAME KCalendarCoreExtras)

target_sources(KCalendarCoreExtras PRIVATE
    calendarlistmodel.cpp
    calendarpluginloader.cpp
)

if (ANDROID)
    add_subdirectory(android)
    target_sources(KCalendarCoreExtras PRIVATE
        androidcalendar.cpp
        androidcalendarplugin.cpp
        androidicalconverter.cpp
    )
    target_link_libraries(KCalendarCoreExtras PRIVATE KAndroidExtras LibIcal)
endif()

target_link_libraries(KCalendarCoreExtras PUBLIC KF5::CalendarCore)

install(TARGETS KCalendarCoreExtras ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
