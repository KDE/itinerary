# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
# SPDX-License-Identifier: BSD-3-Clause

if(ANDROID)
    add_library(SolidExtras)
else()
    add_library(SolidExtras STATIC)
endif()

target_sources(SolidExtras PRIVATE
    brightnessmanager.cpp
    lockmanager.cpp
)

ecm_add_qml_module(SolidExtras GENERATE_PLUGIN_SOURCE
    URI org.kde.solidextras
)

if (ANDROID)
    add_subdirectory(android)
    target_sources(SolidExtras PRIVATE
        androidbrightnessbackend.cpp
        androidlockbackend.cpp
    )
else()
    qt_add_dbus_interface(SolidExtras_SRCS org.kde.Solid.PowerManagement.Actions.BrightnessControl.xml brightnesscontroldbusinterface)
    qt_add_dbus_interface(SolidExtras_SRCS org.freedesktop.ScreenSaver.xml screensaverdbusinterface)
    target_sources(SolidExtras PRIVATE
        solidbrightnessbackend.cpp
        freedesktoplockbackend.cpp
        ${SolidExtras_SRCS}
    )
endif()

target_link_libraries(SolidExtras PUBLIC Qt::Core)

if (NOT ANDROID)
    target_link_libraries(SolidExtras PRIVATE Qt::DBus)
else()
    install(TARGETS SolidExtras ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
endif()
