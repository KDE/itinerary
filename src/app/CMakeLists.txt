if (TARGET KF5::Notifications)
    SET(HAVE_NOTIFICATIONS TRUE)
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-itinerary.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-itinerary.h)

set(itinerary_srcs
    applicationcontroller.cpp
    contenttypeprober.cpp
    countryinformation.cpp
    pkpassmanager.cpp
    pkpassimageprovider.cpp
    reservationmanager.cpp
    timelinemodel.cpp
    tripgroup.cpp
    tripgroupmanager.cpp
    tripgroupproxymodel.cpp
)
ecm_qt_declare_logging_category(itinerary_srcs
    HEADER logging.h
    IDENTIFIER Log
    CATEGORY_NAME org.kde.itinerary
)
add_library(itinerary STATIC ${itinerary_srcs})
target_link_libraries(itinerary PUBLIC
    itinerary-weather
    KPim::Itinerary
    KPim::PkPass
    KF5::I18n
    Qt5::Network
    Qt5::Quick
)

if (Qt5QuickCompiler_FOUND)
    qtquick_compiler_add_resources(qml_srcs qml.qrc)
else ()
    set(qml_srcs qml.qrc)
endif()

set(itinerary_app_srcs
    main.cpp
    countrymodel.cpp
    localizer.cpp
    settings.cpp
    tickettokenmodel.cpp
    util.cpp
    weatherforecastmodel.cpp
    ${qml_srcs}
    brightnessmanager.cpp
    lockmanager.cpp
    livedatamanager.cpp
    journeyquerymodel.cpp
    publictransport.cpp
)
if (ANDROID)
    list(APPEND itinerary_app_srcs
        androidbrightnessbackend.cpp
        androidcontentfileengine.cpp
        androidlockbackend.cpp
    )
else()
    list(APPEND itinerary_app_srcs
        solidbrightnessbackend.cpp
        solidlockbackend.cpp
    )
    qt5_add_dbus_interface(itinerary_app_srcs org.kde.Solid.PowerManagement.Actions.BrightnessControl.xml brightnesscontroldbusinterface)
endif()

add_executable(itinerary-app ${itinerary_app_srcs})
target_include_directories(itinerary-app PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(itinerary-app PRIVATE
    itinerary
    KF5::Contacts
    KF5::CoreAddons
    KPublicTransport
)
if (TARGET KF5::Notifications)
    target_link_libraries(itinerary-app PRIVATE KF5::Notifications)
endif()
if (ANDROID)
    target_include_directories(itinerary-app PRIVATE ${Qt5Core_PRIVATE_INCLUDE_DIRS})
    # explicitly add runtime dependencies and transitive link dependencies,
    # so androiddeployqt picks them up
    target_link_libraries(itinerary PUBLIC Qt5::AndroidExtras)
    target_link_libraries(itinerary-app PRIVATE
        KF5::Archive
        KF5::Kirigami2
        Qt5::Svg
        KF5::Prison
        OpenSSL::SSL
    )

    kirigami_package_breeze_icons(ICONS
        checkmark
        clock
        dialog-cancel
        document-edit
        document-open
        document-save
        documentinfo
        edit-delete
        edit-download
        edit-paste
        go-down-symbolic
        go-home
        go-next-symbolic
        go-up-symbolic
        help-about
        map-symbolic
        meeting-attending
        settings-configure
        view-calendar-day
        view-refresh

        weather-clear
        weather-clear-night
        weather-few-clouds
        weather-few-clouds-night
        weather-clouds
        weather-clouds-night
        weather-showers-day
        weather-showers-night
        weather-showers-scattered-day
        weather-showers-scattered-night
        weather-snow-scattered-day
        weather-snow-scattered-night
        weather-storm-day
        weather-storm-night
        weather-many-clouds
        weather-fog
        weather-showers
        weather-showers-scattered
        weather-hail
        weather-snow
        weather-snow-scattered
        weather-storm
    )
else ()
    target_link_libraries(itinerary PRIVATE Qt5::Positioning Qt5::DBus)
    target_link_libraries(itinerary-app PRIVATE
        KF5::DBusAddons
        Qt5::Widgets
    )
    set_target_properties(itinerary-app PROPERTIES OUTPUT_NAME "itinerary")
endif()

install(TARGETS itinerary-app ${INSTALL_TARGETS_DEFAULT_ARGS})
if (NOT ANDROID)
    install(PROGRAMS org.kde.itinerary.desktop DESTINATION ${KDE_INSTALL_APPDIR})
endif()
install(FILES org.kde.itinerary.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
