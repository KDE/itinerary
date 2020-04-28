if (TARGET KF5::Notifications)
    set(HAVE_NOTIFICATIONS TRUE)
endif()
if (${KPimItinerary_VERSION} VERSION_GREATER 5.13.40)
    set(HAVE_KITINERARY_WITH_VERSION_HEADER TRUE)
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-itinerary.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-itinerary.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/itinerary_version_detailed.h.in ${CMAKE_CURRENT_BINARY_DIR}/itinerary_version_detailed.h)

set(itinerary_srcs
    applicationcontroller.cpp
    documentmanager.cpp
    favoritelocationmodel.cpp
    importexport.cpp
    json.cpp
    livedatamanager.cpp
    localizer.cpp
    locationhelper.cpp
    locationinformation.cpp
    navigationcontroller.cpp
    pkpassmanager.cpp
    pkpassimageprovider.cpp
    publictransport.cpp
    reservationmanager.cpp
    statisticsmodel.cpp
    statisticstimerangemodel.cpp
    timelinedelegatecontroller.cpp
    timelineelement.cpp
    timelinemodel.cpp
    tripgroup.cpp
    tripgroupinfoprovider.cpp
    tripgroupmanager.cpp
    tripgroupproxymodel.cpp
    transfer.cpp
    transfermanager.cpp
    util.cpp
)
ecm_qt_declare_logging_category(itinerary_srcs
    HEADER logging.h
    IDENTIFIER Log
    CATEGORY_NAME org.kde.itinerary
)
add_library(itinerary STATIC ${itinerary_srcs})
target_link_libraries(itinerary PUBLIC
    itinerary-weather
    SolidExtras
    KPublicTransport
    KPim::Itinerary
    KPim::PkPass
    KF5::I18n
    KF5::Contacts
    KF5::CoreAddons
    Qt5::Network
    Qt5::Quick
)
if (TARGET KF5::Notifications)
    target_link_libraries(itinerary PUBLIC KF5::Notifications)
endif()

if (Qt5QuickCompiler_FOUND)
    qtquick_compiler_add_resources(qml_srcs qml.qrc)
else ()
    set(qml_srcs qml.qrc)
endif()

set(itinerary_app_srcs
    main.cpp
    countrymodel.cpp
    documentsmodel.cpp
    settings.cpp
    tickettokenmodel.cpp
    weatherforecastmodel.cpp
    ${qml_srcs}
)

add_executable(itinerary-app ${itinerary_app_srcs})
target_include_directories(itinerary-app PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(itinerary-app PRIVATE
    itinerary
    Qt5::QuickControls2
)
if (ANDROID)
    target_include_directories(itinerary-app PRIVATE ${Qt5Core_PRIVATE_INCLUDE_DIRS})
    # explicitly add runtime dependencies and transitive link dependencies,
    # so androiddeployqt picks them up
    target_link_libraries(itinerary PUBLIC Qt5::AndroidExtras KAndroidExtras)
    target_link_libraries(itinerary-app PRIVATE
        KF5::Archive
        KF5::Kirigami2
        Qt5::Svg
        KF5::Prison
        OpenSSL::SSL
    )

    kirigami_package_breeze_icons(ICONS
        application-pdf
        channel-insecure-symbolic
        channel-secure-symbolic
        checkmark
        clock
        crosshairs
        dialog-cancel
        dialog-close
        document-edit
        document-open
        document-save
        documentinfo
        edit-delete
        edit-download
        edit-paste
        edit-rename
        export-symbolic
        folder-documents-symbolic
        go-down-symbolic
        go-home-symbolic
        go-next-symbolic
        go-up-symbolic
        help-about-symbolic
        help-contents
        list-add
        map-symbolic
        meeting-attending
        question
        settings-configure
        view-calendar-day
        view-list-symbolic
        view-refresh
        view-statistics

        weather-clear
        weather-clear-wind
        weather-clear-night
        weather-clear-wind-night
        weather-few-clouds
        weather-few-clouds-wind
        weather-few-clouds-night
        weather-few-clouds-wind-night
        weather-clouds
        weather-clouds-wind
        weather-clouds-night
        weather-clouds-wind-night
        weather-showers-day
        weather-showers-night
        weather-showers-scattered-day
        weather-showers-scattered-night
        weather-snow-scattered-day
        weather-snow-scattered-night
        weather-storm-day
        weather-storm-night
        weather-many-clouds
        weather-many-clouds-wind
        weather-fog
        weather-showers
        weather-showers-scattered
        weather-hail
        weather-snow
        weather-snow-scattered
        weather-storm
    )
else ()
    target_link_libraries(itinerary PRIVATE Qt5::Positioning Qt5::DBus Qt5::Widgets)
    target_link_libraries(itinerary-app PRIVATE
        KF5::DBusAddons
        Qt5::Widgets
    )
    set_target_properties(itinerary-app PROPERTIES OUTPUT_NAME "itinerary")
endif()

install(TARGETS itinerary-app ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.itinerary.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.itinerary.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
ecm_install_icons(ICONS 48-apps-itinerary.svg DESTINATION ${KDE_INSTALL_ICONDIR})
