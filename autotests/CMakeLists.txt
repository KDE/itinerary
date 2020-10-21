# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

ecm_add_test(jnisignaturetest.cpp LINK_LIBRARIES Qt5::Test)

ecm_add_test(pkpassmanagertest.cpp LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(reservationmanagertest.cpp LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(applicationcontrollertest.cpp LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(tripgrouptest.cpp LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(locationinformationtest.cpp LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(timelinemodeltest.cpp modelverificationpoint.cpp TEST_NAME timelinemodeltest LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(tripgroupproxytest.cpp modelverificationpoint.cpp TEST_NAME tripgroupproxytest LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(tripgroupinfoprovidertest.cpp TEST_NAME tripgroupinfoprovidertest LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(publictransporttest.cpp TEST_NAME publictransporttest LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(timelinedelegatecontrollertest.cpp TEST_NAME timelinedelegatecontrollertest LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(documentmanagertest.cpp TEST_NAME documentmanagertest LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(statisticstest.cpp LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(favoritelocationtest.cpp LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(transfertest.cpp LINK_LIBRARIES Qt5::Test itinerary)
ecm_add_test(livedatamanagertest.cpp LINK_LIBRARIES Qt5::Test itinerary)

ecm_add_test(weathertest.cpp LINK_LIBRARIES Qt5::Test itinerary-weather)
target_include_directories(weathertest PRIVATE ${CMAKE_BINARY_DIR})
