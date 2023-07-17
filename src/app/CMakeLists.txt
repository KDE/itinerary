# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-itinerary.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-itinerary.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/itinerary_version_detailed.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/itinerary_version_detailed.h)

add_library(itinerary STATIC)
target_sources(itinerary PRIVATE
    applicationcontroller.cpp
    calendarhelper.cpp
    calendarimportmodel.cpp
    clipboard.cpp
    documentmanager.cpp
    documentsmodel.cpp
    downloadjob.cpp
    favoritelocationmodel.cpp
    filehelper.cpp
    genericpkpass.cpp
    gpxexport.cpp
    healthcertificatemanager.cpp
    importexport.cpp
    journeysectionmodel.cpp
    json.cpp
    jsonio.cpp
    livedata.cpp
    livedatamanager.cpp
    localizer.cpp
    locationhelper.cpp
    locationinformation.cpp
    navigationcontroller.cpp
    notificationhelper.cpp
    onlineticketretrievaljob.cpp
    passmanager.cpp
    pkpassmanager.cpp
    pkpassimageprovider.cpp
    publictransport.cpp
    publictransportmatcher.cpp
    reservationhelper.cpp
    reservationmanager.cpp
    statisticsmodel.cpp
    statisticstimerangemodel.cpp
    timelinedelegatecontroller.cpp
    timelineelement.cpp
    timelinemodel.cpp
    timelinesectiondelegatecontroller.cpp
    tripgroup.cpp
    tripgroupinfoprovider.cpp
    tripgroupmanager.cpp
    tripgroupproxymodel.cpp
    transfer.cpp
    transfermanager.cpp
    util.cpp
    weatherinformation.cpp
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
    KPublicTransportOnboard
    KOSMIndoorMap
    KPim${KF_MAJOR_VERSION}::Itinerary
    KPim${KF_MAJOR_VERSION}::PkPass
    KF${KF_MAJOR_VERSION}::CalendarCore
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::I18nLocaleData
    KF${KF_MAJOR_VERSION}::Contacts
    KF${KF_MAJOR_VERSION}::CoreAddons
    KF${KF_MAJOR_VERSION}::Notifications
    KF${KF_MAJOR_VERSION}::Holidays
    Qt::Network
    Qt::Quick
)
if (TARGET KF5::Mime)
    target_link_libraries(itinerary PUBLIC KF5::Mime)
else()
    target_link_libraries(itinerary PUBLIC KPim${KF_MAJOR_VERSION}::Mime)
endif()
if (HAVE_MATRIX)
    target_link_libraries(itinerary PUBLIC KMatrix)
endif()

if (TARGET KHealthCertificate)
    target_link_libraries(itinerary PRIVATE KHealthCertificate)
endif()

configure_file(EventTicket.qml ${CMAKE_CURRENT_BINARY_DIR}/EventTicket.qml)
configure_file(main.qml ${CMAKE_CURRENT_BINARY_DIR}/main.qml)
configure_file(FormPlaceEditorDelegate.qml ${CMAKE_CURRENT_BINARY_DIR}/FormPlaceEditorDelegate.qml)
configure_file(LocationPicker.qml ${CMAKE_CURRENT_BINARY_DIR}/LocationPicker.qml)
configure_file(LiveStatusPage.qml ${CMAKE_CURRENT_BINARY_DIR}/LiveStatusPage.qml)
configure_file(FavoriteLocationPage.qml ${CMAKE_CURRENT_BINARY_DIR}/FavoriteLocationPage.qml)

configure_file(qml.qrc ${CMAKE_CURRENT_SOURCE_DIR}/qml.generated.qrc)

if (Qt5QuickCompiler_FOUND)
    qtquick_compiler_add_resources(qml_srcs qml.generated.qrc)
else ()
    set(qml_srcs qml.generated.qrc)
endif()

add_executable(itinerary-app ${itinerary_app_srcs})
target_sources(itinerary-app PRIVATE
    main.cpp
    countrysubdivisionmodel.cpp
    developmentmodecontroller.cpp
    factory.cpp
    locationinformationdelegatecontroller.cpp
    mapdownloadmanager.cpp
    matrixcontroller.cpp
    notificationconfigcontroller.cpp
    notifyrc.qrc
    onlineticketimporter.cpp
    permissionmanager.cpp
    settings.cpp
    tickettokenmodel.cpp
    transferdelegatecontroller.cpp
    weatherforecastmodel.cpp
    ${qml_srcs}
    )
target_include_directories(itinerary-app PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(itinerary-app PRIVATE
    itinerary
    Qt::QuickControls2
    KCalendarCoreExtras
)
if (ANDROID)
    target_include_directories(itinerary-app PRIVATE ${Qt5Core_PRIVATE_INCLUDE_DIRS})
    target_link_libraries(itinerary PUBLIC KAndroidExtras)
    if (QT_MAJOR_VERSION EQUAL "6")
        target_link_libraries(itinerary PRIVATE Qt::CorePrivate)
        target_link_libraries(itinerary-app PRIVATE Qt::CorePrivate)
    endif()
    # explicitly add runtime dependencies and transitive link dependencies,
    # so androiddeployqt picks them up
    target_link_libraries(itinerary-app PRIVATE
        KF${KF_MAJOR_VERSION}::Archive
        KF${KF_MAJOR_VERSION}::Kirigami2
        Qt::Svg
        KF${KF_MAJOR_VERSION}::Prison
        OpenSSL::SSL
    )

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/android/version.gradle.in ${CMAKE_BINARY_DIR}/version.gradle)

    kirigami_package_breeze_icons(ICONS
        application-pdf
        bookmarks
        call-start
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
        emblem-shared-symbolic
        export-symbolic
        flag
        flashlight-off
        flashlight-on
        folder-documents-symbolic
        globe
        go-down-symbolic
        go-home-symbolic
        go-next-symbolic
        go-up-symbolic
        help-about-symbolic
        help-contents
        list-add
        mail-message-new
        media-playback-playing
        map-globe
        map-symbolic
        meeting-attending
        notifications
        question
        raindrop
        search
        settings-configure
        temperature-cold
        temperature-normal
        temperature-warm
        view-barcode-qr
        view-calendar-day
        view-list-symbolic
        view-list-details
        view-refresh
        view-statistics
        wallet-open
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

    install(FILES sc-apps-org.kde.itinerary.svg  DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kirigami.2/icons/ RENAME org.kde.itinerary.svg)
else ()
    target_link_libraries(itinerary PRIVATE Qt::Positioning Qt::DBus Qt::Widgets)
    target_link_libraries(itinerary-app PRIVATE
        KF${KF_MAJOR_VERSION}::DBusAddons
        KF${KF_MAJOR_VERSION}::WindowSystem
        Qt::Widgets
    )
    set_target_properties(itinerary-app PROPERTIES OUTPUT_NAME "itinerary")
    ecm_install_icons(ICONS sc-apps-org.kde.itinerary.svg DESTINATION ${KDE_INSTALL_ICONDIR})
endif()
if (TARGET KF${KF_MAJOR_VERSION}::Crash)
    target_link_libraries(itinerary-app PRIVATE KF${KF_MAJOR_VERSION}::Crash)
endif()
if (TARGET KF${KF_MAJOR_VERSION}::UnitConversion)
    target_link_libraries(itinerary-app PRIVATE KF${KF_MAJOR_VERSION}::UnitConversion)
endif()

install(TARGETS itinerary-app ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
if (NOT ANDROID)
    install(PROGRAMS org.kde.itinerary.desktop DESTINATION ${KDE_INSTALL_APPDIR})
    install(FILES org.kde.itinerary.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
    install(FILES itinerary.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
endif()
