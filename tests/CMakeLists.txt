add_executable(pkpassviewer pkpassviewer.cpp pkpassviewer.qrc)
target_link_libraries(pkpassviewer itinerary)

add_executable(journeyquery journeyquery.cpp journeyquery.qrc)
target_link_libraries(journeyquery KPublicTransport Qt5::Quick)
