add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

ecm_add_test(pkpasstest.cpp LINK_LIBRARIES Qt5::Test KPkPass)
ecm_add_test(pkpassmanagertest.cpp LINK_LIBRARIES Qt5::Test itinerary)
