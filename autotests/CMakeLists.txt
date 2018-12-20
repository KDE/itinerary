add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

ecm_add_test(pkpassmanagertest.cpp LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(reservationmanagertest.cpp LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(applicationcontrollertest.cpp LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(tripgrouptest.cpp LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(timelinemodeltest.cpp modelverificationpoint.cpp TEST_NAME timelinemodeltest LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(tripgroupproxytest.cpp modelverificationpoint.cpp TEST_NAME tripgroupproxytest LINK_LIBRARIES Qt5::Test itinerary)

ecm_add_test(weathertest.cpp LINK_LIBRARIES Qt5::Test itinerary-weather)
target_include_directories(weathertest PRIVATE ${CMAKE_BINARY_DIR})

ecm_add_test(navitiaparsertest.cpp LINK_LIBRARIES Qt5::Test KPublicTransport)
ecm_add_test(hafasparsertest.cpp LINK_LIBRARIES Qt5::Test KPublicTransport)
ecm_add_test(publictransportmanagertest.cpp LINK_LIBRARIES Qt5::Test KPublicTransport)
