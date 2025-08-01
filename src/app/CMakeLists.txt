# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-itinerary.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-itinerary.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/itinerary_version_detailed.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/itinerary_version_detailed.h)

add_library(itinerary STATIC)
target_sources(itinerary PRIVATE
    applicationcontroller.cpp
    calendarhelper.cpp
    clipboard.cpp
    costaccumulator.cpp
    countrysubdivisionmodel.cpp
    documentmanager.cpp
    documentsmodel.cpp
    downloadjob.cpp
    favoritelocationmodel.cpp
    filehelper.cpp
    genericpkpass.cpp
    gpxexport.cpp
    healthcertificatemanager.cpp
    importcontroller.cpp
    importexport.cpp
    intenthandler.cpp
    journeysectionmodel.cpp
    json.cpp
    jsonio.cpp
    kdeconnect.cpp
    livedata.cpp
    livedatamanager.cpp
    localizer.cpp
    locationhelper.cpp
    locationinformation.cpp
    mapdownloadmanager.cpp
    matrixcontroller.cpp
    navigationcontroller.cpp
    nominatimimportjob.cpp
    notificationhelper.cpp
    onlineticketimporter.cpp
    onlineticketretrievaljob.cpp
    osmimportjob.cpp
    passmanager.cpp
    permissionmanager.cpp
    pkpassimageprovider.cpp
    pkpassmanager.cpp
    publictransport.cpp
    publictransportmatcher.cpp
    qmlsingletons.h
    reservationhelper.cpp
    reservationmanager.cpp
    settings.cpp
    statisticsmodel.cpp
    statisticstimerangemodel.cpp
    tickettokenmodel.cpp
    timelinedelegatecontroller.cpp
    timelineelement.cpp
    timelinemodel.cpp
    timelinesectiondelegatecontroller.cpp
    traewellingcontroller.cpp
    transfer.cpp
    transferdelegatecontroller.cpp
    transfermanager.cpp
    tripgroup.cpp
    tripgroupcontroller.cpp
    tripgroupfilterproxymodel.cpp
    tripgrouplocationmodel.cpp
    tripgroupmanager.cpp
    tripgroupmapmodel.cpp
    tripgroupmodel.cpp
    tripgroupsplitmodel.cpp
    util.cpp
    weatherforecastmodel.cpp
    weatherinformation.cpp
)

ecm_add_qml_module(itinerary GENERATE_PLUGIN_SOURCE
    URI org.kde.itinerary
    IMPORTS
        org.kde.itinerary.weather
        org.kde.itinerary.matrix
    QML_FILES
        Main.qml
        AboutPage.qml
        ActionsCard.qml
        AlternativeJourneyPage.qml
        BarcodeContainer.qml
        BarcodeScanModeController.qml
        BarcodeScanModeButton.qml
        BarcodeScannerPage.qml
        BoardingPass.qml
        BoatDelegate.qml
        BoatEditor.qml
        BoatPage.qml
        BookingCard.qml
        BookingEditorCard.qml
        BusDelegate.qml
        BusEditor.qml
        BusPage.qml
        CardPageTitle.qml
        CalendarSelectionDialog.qml
        CarRentalDelegate.qml
        CarRentalPage.qml
        ContactCard.qml
        ContactEditorCard.qml
        CountryComboBoxDelegate.qml
        DepartureQueryPage.qml
        DetailsPage.qml
        DevelopmentModePage.qml
        DocumentsCard.qml
        EditorPage.qml
        EventDelegate.qml
        EventEditor.qml
        EventPage.qml
        EventTicket.qml
        ExportMenuDialog.qml
        FavoriteLocationPage.qml
        FlightDelegate.qml
        FlightEditor.qml
        FlightPage.qml
        FormDateTimeEditDelegate.qml
        FormPlaceDelegate.qml
        FormPlaceEditorDelegate.qml
        FormPlatformDelegate.qml
        FormPriceDelegate.qml
        FormPriceEditDelegate.qml
        GenericPass.qml
        GenericPkPassPage.qml
        HealthCertificateBarcode.qml
        HealthCertificateRecovery.qml
        HealthCertificatePage.qml
        HealthCertificateTest.qml
        HealthCertificateVaccination.qml
        HotelDelegate.qml
        HotelEditor.qml
        HotelPage.qml
        ImportAction.qml
        ImportPage.qml
        IndoorMapGateSheet.qml
        IndoorMapInfoSheet.qml
        IndoorMapPage.qml
        IntermediateStopSelector.qml
        JourneyDelegateHeader.qml
        JourneyPathPage.qml
        JourneyRequestPage.qml
        JourneyQueryPage.qml
        JourneySectionDelegate.qml
        JourneySectionMapView.qml
        JourneySectionPage.qml
        JourneySectionStopDelegate.qml
        JourneySectionStopDelegateLineSegment.qml
        JourneySummaryDelegate.qml
        LiveStatusPage.qml
        LocationInfoDelegate.qml
        LocationPicker.qml
        MapCircle.qml
        MapPin.qml
        MapStopoverInfoSheetDrawer.qml
        MapView.qml
        MatrixRoomSelectionSheet.qml
        MatrixKeyVerificationPage.qml
        NavigationBar.qml
        MyDataPage.qml
        OnlineImportPage.qml
        PassPage.qml
        PassDelegate.qml
        PkPassBarcode.qml
        PkPassPage.qml
        ProgramMembershipCard.qml
        ProgramMembershipEditorCard.qml
        ProgramMembershipEditor.qml
        ProgramMembershipPage.qml
        ReservationDocumentsCard.qml
        RestaurantDelegate.qml
        RestaurantEditor.qml
        RestaurantPage.qml
        SeatRowFormCard.qml
        SettingsPage.qml
        SheetDrawer.qml
        StatisticsDelegate.qml
        StatisticsPage.qml
        StopoverDetailsPage.qml
        StopoverInformationSheet.qml
        StopoverFormDelegate.qml
        StopPickerPage.qml
        TicketEditor.qml
        TicketPage.qml
        TicketTokenDelegate.qml
        TicketTokenEditorCard.qml
        TimelineDelegateArrivalLayout.qml
        TimelineDelegate.qml
        TimelineDelegateDepartureLayout.qml
        TimelineDelegateIntermediateStopsButton.qml
        TimelineDelegateIntermediateStopsView.qml
        TimelineDelegateSeatRow.qml
        TimelineDelegateSeatRowLabel.qml
        TimelineSectionDelegate.qml
        TouristAttractionDelegate.qml
        TouristAttractionPage.qml
        TrainDelegate.qml
        TrainEditor.qml
        TrainPage.qml
        TransferDelegate.qml
        TransferPage.qml
        TripGroupEditorDialog.qml
        TripGroupMapPage.qml
        TripGroupMergeDialog.qml
        TripGroupPage.qml
        TripGroupSelectorCard.qml
        TripGroupSplitPage.qml
        TripGroupsPage.qml
        VehicleLayoutPage.qml
        VehicleSectionDialog.qml
        VerticalNavigationButton.qml
        WeatherForecastDelegate.qml
        WeatherForecastPage.qml
        WelcomePage.qml

        onlineimport/db.qml
        onlineimport/sncf.qml
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
    itinerary-weatherplugin
    SolidExtras
    GpxIo
    KPublicTransport
    KPublicTransportOnboard
    KOSMIndoorMap
    KPim6::Itinerary
    KPim6::PkPass
    KPim6::Mime
    KF6::CalendarCore
    KF6::I18n
    KF6::I18nLocaleData
    KF6::Contacts
    KF6::CoreAddons
    KF6::Notifications
    KF6::Holidays
    KF6::UnitConversion
    Qt::Network
    Qt::Quick
    Qt::Positioning
PRIVATE
    Qt::CorePrivate
)
if (HAVE_MATRIX)
    target_link_libraries(itinerary PUBLIC KMatrix)
endif()

if (TARGET KHealthCertificate)
    target_link_libraries(itinerary PRIVATE KHealthCertificate)
endif()

add_executable(itinerary-app ${itinerary_app_srcs})
target_sources(itinerary-app PRIVATE
    main.cpp
    developmentmodecontroller.cpp
    factory.cpp
    migrator.cpp
    notificationconfigcontroller.cpp
    notifyrc.qrc
    unitconversion.cpp

    images/images.qrc
)
target_include_directories(itinerary-app PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(itinerary-app PRIVATE
    itinerary
    itineraryplugin
    KF6::I18nQml
    Qt::QuickControls2
    KCalendarCoreExtras
    Qt6Keychain::Qt6Keychain
    KMatrixplugin
)
if (ANDROID)
    target_link_libraries(itinerary PUBLIC KAndroidExtras PRIVATE Qt::CorePrivate)
    # explicitly add runtime dependencies and transitive link dependencies,
    # so androiddeployqt picks them up
    target_link_libraries(itinerary-app PRIVATE
        KF6::Archive
        KF6::ColorScheme
        KF6::Kirigami
        Qt::Svg
        Qt::CorePrivate
        KF6::Prison
        OpenSSL::SSL
    )

    kirigami_package_breeze_icons(ICONS
        ${KIRIGAMI_ADDONS_ICONS}
        ${KPUBLICTRANSPORT_ICONS}

        applications-development-symbolic
        application-pdf
        arrow-down
        arrow-left
        arrow-right
        arrow-up
        backup
        bookmarks
        call-start
        channel-insecure-symbolic
        channel-secure-symbolic
        checkmark
        clock
        cloud-upload
        crosshairs
        cross-shape
        dialog-cancel
        dialog-close
        dialog-error-symbolic
        dialog-ok
        document-edit
        document-export-symbolic
        document-import-symbolic
        document-open
        document-save
        document-save-as-symbolic
        documentinfo
        donate-symbolic
        download
        edit-copy-symbolic
        edit-cut
        edit-delete
        edit-download
        edit-paste
        edit-rename
        emblem-important
        emblem-shared-symbolic
        flag
        flashlight-off
        flashlight-on
        folder-documents-symbolic
        format-list-unordered
        globe-symbolic
        go-down-symbolic
        go-home-symbolic
        go-next-symbolic
        go-up-symbolic
        help-about-symbolic
        help-contents
        license
        list-add
        mail-attachment-symbolic
        mail-message-new
        map-globe
        map-gnomonic
        map-symbolic
        media-playback-playing
        media-playback-start
        media-playback-stop
        meeting-attending
        merge
        message-rfc822
        network-disconnect-symbolic
        notifications
        question
        raindrop
        reverse
        search
        security-high
        security-medium-symbolic
        security-low
        settings-configure
        split
        temperature-cold
        temperature-normal
        temperature-warm
        tools-report-bug-symbolic
        view-barcode-qr
        view-calendar
        view-calendar-day
        view-currency-list
        view-list-details
        view-refresh
        view-statistics-symbolic
        wallet-open
        window-close-symbolic
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

    ecm_add_android_apk(itinerary-app ANDROID_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android)
    install(FILES sc-apps-org.kde.itinerary.svg  DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kirigami/breeze-internal/icons/ RENAME org.kde.itinerary.svg)
else ()
    target_link_libraries(itinerary PRIVATE Qt::Positioning Qt::DBus Qt::Widgets)
    target_link_libraries(itinerary-app PRIVATE
        KF6::DBusAddons
        KF6::WindowSystem
        Qt::Widgets
    )
    set_target_properties(itinerary-app PROPERTIES OUTPUT_NAME "itinerary")
    ecm_install_icons(ICONS sc-apps-org.kde.itinerary.svg DESTINATION ${KDE_INSTALL_ICONDIR})
endif()
if (TARGET KF6::Crash)
    target_link_libraries(itinerary-app PRIVATE KF6::Crash)
endif()

install(TARGETS itinerary-app ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
if (NOT ANDROID)
    install(PROGRAMS org.kde.itinerary.desktop DESTINATION ${KDE_INSTALL_APPDIR})
    install(FILES org.kde.itinerary.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
    install(FILES itinerary.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
endif()
