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

add_executable(itinerary-app
    main.cpp
    applicationcontroller.cpp
    countrymodel.cpp
    localizer.cpp
    pkpassimageprovider.cpp
    settings.cpp
    qml.qrc
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
    )

    kirigami_package_breeze_icons(ICONS
        document-open
        edit-delete
        go-next-symbolic
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
    target_link_libraries(itinerary-app PRIVATE Qt5::Positioning)
    set_target_properties(itinerary-app PROPERTIES OUTPUT_NAME "itinerary")
endif()

qml_lint(
    main.qml
    BoardingPass.qml
    BusDelegate.qml
    BusPage.qml
    CountryInfoDelegate.qml
    DetailsPage.qml
    FlightDelegate.qml
    FlightPage.qml
    HotelDelegate.qml
    HotelPage.qml
    ImportDialog.qml
    PkPassPage.qml
    PlaceDelegate.qml
    RestaurantDelegate.qml
    RestaurantPage.qml
    SettingsPage.qml
    TicketTokenDelegate.qml
    TimelineDelegate.qml
    TimelinePage.qml
    TouristAttractionDelegate.qml
    TrainDelegate.qml
    TrainPage.qml
    WeatherForecastDelegate.qml

    +android/ImportDialog.qml
)

install(TARGETS itinerary-app ${INSTALL_TARGETS_DEFAULT_ARGS})
if (NOT ANDROID)
    install(PROGRAMS org.kde.itinerary.desktop DESTINATION ${KDE_INSTALL_APPDIR})
endif()
