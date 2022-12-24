# SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

gradle_add_aar(calendarextras_aar BUILDFILE ${CMAKE_CURRENT_SOURCE_DIR}/build.gradle NAME KCalendarCoreExtras)
gradle_install_aar(calendarextras_aar DESTINATION jar)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/KCalendarCoreExtras-android-dependencies.xml
    ${CMAKE_BINARY_DIR}/lib/KCalendarCoreExtras_${CMAKE_ANDROID_ARCH_ABI}-android-dependencies.xml
)
install(FILES KCalendarCoreExtras-android-dependencies.xml DESTINATION ${KDE_INSTALL_LIBDIR} RENAME KCalendarCoreExtras_${CMAKE_ANDROID_ARCH_ABI}-android-dependencies.xml)
