add_library(itinerary STATIC
    pkpassmanager.cpp
    timelinemodel.cpp
)
target_link_libraries(itinerary PUBLIC
    KPkPass
)

add_executable(itinerary-app
    main.cpp
    pkpassimageprovider.cpp
    qml.qrc
)
target_include_directories(itinerary-app PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(itinerary-app PRIVATE
    itinerary
    Qt5::Quick
)
set_target_properties(itinerary-app PROPERTIES OUTPUT_NAME "itinerary")

qml_lint(
    main.qml
    BoardingPass.qml
    TimelinePage.qml
)

install(TARGETS itinerary-app ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.itinerary.desktop DESTINATION ${KDE_INSTALL_APPDIR})
