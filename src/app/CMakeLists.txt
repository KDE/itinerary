# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-itinerary.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-itinerary.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/itinerary_version_detailed.h.in ${CMAKE_CURRENT_BINARY_DIR}/itinerary_version_detailed.h)

add_library(itinerary STATIC)
target_sources(itinerary PRIVATE
    applicationcontroller.cpp
    documentmanager.cpp
    favoritelocationmodel.cpp
    gpxexport.cpp
    healthcertificatemanager.cpp
    importexport.cpp
    json.cpp
    livedata.cpp
    livedatamanager.cpp
    localizer.cpp
    locationhelper.cpp
    locationinformation.cpp
    navigationcontroller.cpp
    notificationhelper.cpp
    pkpassmanager.cpp
    pkpassimageprovider.cpp
    publictransport.cpp
    reservationhelper.cpp
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
ecm_qt_declare_logging_category(itinerary
    HEADER logging.h
    IDENTIFIER Log
    CATEGORY_NAME org.kde.itinerary
    DESCRIPTION "KDE Itinerary app"
    EXPORT ITINERARY 
)

target_link_libraries(itinerary PUBLIC
    itinerary-weather
    SolidExtras
    GpxIo
    KPublicTransport
    KPim::Itinerary
    KPim::PkPass
    KF5::I18n
    KF5::Contacts
    KF5::CoreAddons
    KF5::Notifications
    Qt::Network
    Qt::Quick
)
if (TARGET KHealthCertificate)
    target_link_libraries(itinerary PRIVATE KHealthCertificate)
endif()

if (Qt5QuickCompiler_FOUND)
    qtquick_compiler_add_resources(qml_srcs qml.qrc)
else ()
    set(qml_srcs qml.qrc)
endif()

add_executable(itinerary-app ${itinerary_app_srcs})
target_sources(itinerary-app PRIVATE
    main.cpp
    countrymodel.cpp
    developmentmodecontroller.cpp
    documentsmodel.cpp
    mapdownloadmanager.cpp
    notificationconfigcontroller.cpp
    notifyrc.qrc
    settings.cpp
    tickettokenmodel.cpp
    weatherforecastmodel.cpp
    ${qml_srcs}
    )
target_include_directories(itinerary-app PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(itinerary-app PRIVATE
    itinerary
    KOSMIndoorMap
    Qt::QuickControls2
)
if (ANDROID)
    target_include_directories(itinerary-app PRIVATE ${Qt5Core_PRIVATE_INCLUDE_DIRS})
    # explicitly add runtime dependencies and transitive link dependencies,
    # so androiddeployqt picks them up
    target_link_libraries(itinerary PUBLIC Qt::AndroidExtras KAndroidExtras)
    target_link_libraries(itinerary-app PRIVATE
        KF5::Mime
        KF5::Archive
        KF5::Kirigami2
        Qt::Svg
        KF5::Prison
        OpenSSL::SSL
    )

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/android/version.gradle.in ${CMAKE_BINARY_DIR}/version.gradle)

    kirigami_package_breeze_icons(ICONS
        application-pdf
        channel-insecure-symbolic
        channel-secure-symbolic
        checkmark
        clock
        crosshairs
        cross-shape
        dialog-cancel
        dialog-close
        dialog-error-symbolic
        dialog-ok
        document-edit
        document-import
        document-open
        document-save
        documentinfo
        download
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
        map-globe
        map-symbolic
        meeting-attending
        notifications
        question
        settings-configure
        view-calendar-day
        view-list-symbolic
        view-refresh
        view-statistics
        zoom-in-symbolic
        zoom-out-symbolic

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
    target_link_libraries(itinerary PRIVATE Qt::Positioning Qt::DBus Qt::Widgets)
    target_link_libraries(itinerary-app PRIVATE
        KF5::DBusAddons
        Qt::Widgets
    )
    set_target_properties(itinerary-app PROPERTIES OUTPUT_NAME "itinerary")
endif()
if (TARGET KF5::Crash)
    target_link_libraries(itinerary-app PRIVATE KF5::Crash)
endif()

install(TARGETS itinerary-app ${INSTALL_TARGETS_DEFAULT_ARGS})
ecm_install_icons(ICONS 48-apps-itinerary.svg DESTINATION ${KDE_INSTALL_ICONDIR})
if (NOT ANDROID)
    install(PROGRAMS org.kde.itinerary.desktop DESTINATION ${KDE_INSTALL_APPDIR})
    install(FILES org.kde.itinerary.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
    install(FILES itinerary.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})
endif()
