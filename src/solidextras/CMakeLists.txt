# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(qml)

set(solidextras_srcs
    brightnessmanager.cpp
    lockmanager.cpp
    networkstatus.cpp
)

if (ANDROID)
    add_subdirectory(android)
    list(APPEND solidextras_srcs
        androidbrightnessbackend.cpp
        androidlockbackend.cpp
        networkstatus_android.cpp
    )
else()
    list(APPEND solidextras_srcs
        solidbrightnessbackend.cpp
        freedesktoplockbackend.cpp
        networkstatus_dbus.cpp
    )
    qt_add_dbus_interface(solidextras_srcs org.kde.Solid.PowerManagement.Actions.BrightnessControl.xml brightnesscontroldbusinterface)
    qt_add_dbus_interface(solidextras_srcs org.freedesktop.ScreenSaver.xml screensaverdbusinterface)
    qt_add_dbus_interface(solidextras_srcs org.freedesktop.portal.NetworkMonitor.xml portalnetworkmonitor)
endif()

add_library(SolidExtras ${solidextras_srcs})
generate_export_header(SolidExtras BASE_NAME SolidExtras)

if (TARGET KF6::NetworkManagerQt)
    set(HAVE_NM ON)
endif()
configure_file(config-solid-extras.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-solid-extras.h)

target_include_directories(SolidExtras PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>")
target_link_libraries(SolidExtras PUBLIC Qt::Core)

if (NOT ANDROID)
    target_link_libraries(SolidExtras PRIVATE Qt::DBus)
    if (TARGET KF6::NetworkManagerQt)
        target_link_libraries(SolidExtras PRIVATE KF6::NetworkManagerQt)
    endif()
endif()

install(TARGETS SolidExtras ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
