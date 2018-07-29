set(itinerary_srcs
    countryinformation.cpp
    pkpassmanager.cpp
    reservationmanager.cpp
    timelinemodel.cpp
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
)

if (Qt5QuickCompiler_FOUND)
    qtquick_compiler_add_resources(qml_srcs qml.qrc)
else ()
    set(qml_srcs qml.qrc)
endif()

add_executable(itinerary-app
    main.cpp
    applicationcontroller.cpp
    countrymodel.cpp
    localizer.cpp
    pkpassimageprovider.cpp
    settings.cpp
    tickettokenmodel.cpp
    util.cpp
    ${qml_srcs}
)
target_include_directories(itinerary-app PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(itinerary-app PRIVATE
    itinerary
    Qt5::Quick
    KF5::Contacts
)
if (ANDROID)
    # explicitly add runtime dependencies and transitive link dependencies,
    # so androiddeployqt picks them up
    target_link_libraries(itinerary-app PRIVATE
        KF5::Archive
        KF5::Kirigami2
        Qt5::AndroidExtras
        Qt5::Svg
        KF5::Prison
        OpenSSL::SSL
    )

    kirigami_package_breeze_icons(ICONS
        checkmark
        dialog-cancel
        document-edit
        document-open
        document-save
        edit-delete
        edit-download
        go-next-symbolic
        help-about
        map-symbolic
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
    target_link_libraries(itinerary-app PRIVATE
        KF5::DBusAddons
        Qt5::Positioning
        Qt5::Widgets
    )
    set_target_properties(itinerary-app PROPERTIES OUTPUT_NAME "itinerary")
endif()

install(TARGETS itinerary-app ${INSTALL_TARGETS_DEFAULT_ARGS})
if (NOT ANDROID)
    install(PROGRAMS org.kde.itinerary.desktop DESTINATION ${KDE_INSTALL_APPDIR})
    install(FILES org.kde.itinerary.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
endif()
