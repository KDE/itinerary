add_executable(pkpassviewer pkpassviewer.cpp pkpassviewer.qrc)
target_link_libraries(pkpassviewer itinerary)

add_executable(departurequery departurequery.cpp departurequery.qrc)
target_link_libraries(departurequery KPublicTransport Qt5::Quick Qt5::Widgets)

add_executable(journeyquery journeyquery.cpp journeyquery.qrc)
target_link_libraries(journeyquery KPublicTransport Qt5::Quick Qt5::Widgets)

add_executable(locationquery locationquery.cpp locationquery.qrc)
target_link_libraries(locationquery KPublicTransport Qt5::Quick Qt5::Widgets)
