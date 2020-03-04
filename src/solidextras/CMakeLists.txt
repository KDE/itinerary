set(solidextras_srcs
    brightnessmanager.cpp
    lockmanager.cpp
)

if (ANDROID)
    list(APPEND solidextras_srcs
        androidbrightnessbackend.cpp
        androidlockbackend.cpp
    )
else()
    list(APPEND solidextras_srcs
        solidbrightnessbackend.cpp
        solidlockbackend.cpp
    )
    qt5_add_dbus_interface(solidextras_srcs org.kde.Solid.PowerManagement.Actions.BrightnessControl.xml brightnesscontroldbusinterface)
    qt5_add_dbus_interface(solidextras_srcs org.freedesktop.ScreenSaver.xml screensaverdbusinterface)
endif()

add_library(SolidExtras STATIC ${solidextras_srcs})
target_link_libraries(SolidExtras PUBLIC Qt5::Core)

if (ANDROID)
    target_link_libraries(SolidExtras PRIVATE Qt5::AndroidExtras)
else()
    target_link_libraries(SolidExtras PRIVATE Qt5::DBus)
endif()
