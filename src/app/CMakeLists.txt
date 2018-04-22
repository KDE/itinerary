set(itinerary_srcs
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
    KPim::Itinerary
    KPim::PkPass
    KF5::I18n
    Qt5::Network
)

add_executable(itinerary-app
    main.cpp
    applicationcontroller.cpp
    localizer.cpp
    pkpassimageprovider.cpp
    qml.qrc
)
target_include_directories(itinerary-app PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(itinerary-app PRIVATE
    itinerary
    Qt5::Quick
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
        view-calendar-day
        view-refresh
    )
else ()
    target_link_libraries(itinerary-app PRIVATE Qt5::Positioning)
    set_target_properties(itinerary-app PROPERTIES OUTPUT_NAME "itinerary")
endif()

qml_lint(
    main.qml
    BoardingPass.qml
    BusDelegate.qml
    FlightDelegate.qml
    HotelDelegate.qml
    PkPassPage.qml
    PlaceDelegate.qml
    RestaurantDelegate.qml
    TicketPage.qml
    TimelineDelegate.qml
    TimelinePage.qml
    TrainDelegate.qml
)

install(TARGETS itinerary-app ${INSTALL_TARGETS_DEFAULT_ARGS})
if (NOT ANDROID)
    install(PROGRAMS org.kde.itinerary.desktop DESTINATION ${KDE_INSTALL_APPDIR})
endif()
