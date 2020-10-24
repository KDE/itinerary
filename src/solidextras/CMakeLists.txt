# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

set(solidextras_srcs
    brightnessmanager.cpp
    lockmanager.cpp
)

if (ANDROID)
    add_subdirectory(android)

    list(APPEND solidextras_srcs
        androidbrightnessbackend.cpp
        androidlockbackend.cpp
    )

    add_library(SolidExtras ${solidextras_srcs})
    target_link_libraries(SolidExtras PRIVATE Qt5::AndroidExtras)
    install(TARGETS SolidExtras ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
else()
    list(APPEND solidextras_srcs
        solidbrightnessbackend.cpp
        solidlockbackend.cpp
    )
    qt5_add_dbus_interface(solidextras_srcs org.kde.Solid.PowerManagement.Actions.BrightnessControl.xml brightnesscontroldbusinterface)
    qt5_add_dbus_interface(solidextras_srcs org.freedesktop.ScreenSaver.xml screensaverdbusinterface)

    add_library(SolidExtras STATIC ${solidextras_srcs})
    target_link_libraries(SolidExtras PRIVATE Qt5::DBus)
endif()

generate_export_header(SolidExtras BASE_NAME SolidExtras)
target_link_libraries(SolidExtras PUBLIC Qt5::Core)
