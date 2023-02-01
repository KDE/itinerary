# SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

if (ANDROID)
    add_library(KCalendarCoreExtras)
else()
    add_library(KCalendarCoreExtras STATIC)
endif()
generate_export_header(KCalendarCoreExtras BASE_NAME KCalendarCoreExtras)

target_sources(KCalendarCoreExtras PRIVATE
    calendarlistmodel.cpp
    calendarpluginloader.cpp
    integration.cpp
)

if (ANDROID)
    add_subdirectory(android)
    target_sources(KCalendarCoreExtras PRIVATE
        androidcalendar.cpp
        androidcalendarplugin.cpp
        androidicalconverter.cpp
    )
    target_compile_definitions(KCalendarCoreExtras PRIVATE -DQT_STATICPLUGIN)
    target_link_libraries(KCalendarCoreExtras PRIVATE KAndroidExtras LibIcal)
endif()

target_link_libraries(KCalendarCoreExtras PUBLIC KF${KF_MAJOR_VERSION}::CalendarCore)

if (ANDROID)
    install(TARGETS KCalendarCoreExtras ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
endif()
