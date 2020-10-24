# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

gradle_add_aar(solidextras_aar BUILDFILE ${CMAKE_CURRENT_SOURCE_DIR}/build.gradle NAME SolidExtras)
gradle_install_aar(solidextras_aar DESTINATION jar)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/SolidExtras-android-dependencies.xml
    ${CMAKE_BINARY_DIR}/lib/SolidExtras_${CMAKE_ANDROID_ARCH_ABI}-android-dependencies.xml
)
install(FILES SolidExtras-android-dependencies.xml DESTINATION ${KDE_INSTALL_LIBDIR} RENAME SolidExtras_${CMAKE_ANDROID_ARCH_ABI}-android-dependencies.xml)
