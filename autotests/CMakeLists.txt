# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

ecm_add_test(jnisignaturetest.cpp LINK_LIBRARIES Qt::Test KAndroidExtras)
ecm_add_test(jnipropertytest.cpp LINK_LIBRARIES Qt::Test KAndroidExtras)
ecm_add_test(jniarraytest.cpp LINK_LIBRARIES Qt::Test KAndroidExtras)
ecm_add_test(jnimethodtest.cpp LINK_LIBRARIES Qt::Test KAndroidExtras)
ecm_add_test(androidwrappertest.cpp LINK_LIBRARIES Qt::Test KAndroidExtras)

if (TARGET LibIcal)
    ecm_add_test(androidicalconvertertest.cpp LINK_LIBRARIES Qt::Test KAndroidExtras KCalendarCoreExtras LibIcal)
endif()

ecm_add_test(gpxwritertest.cpp LINK_LIBRARIES Qt::Test GpxIo)

ecm_add_test(pkpassmanagertest.cpp mocknetworkaccessmanager.cpp TEST_NAME pkpassmanagertest LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(reservationmanagertest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(applicationcontrollertest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(importcontrollertest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(tripgrouptest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(locationinformationtest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(timelinemodeltest.cpp modelverificationpoint.cpp TEST_NAME timelinemodeltest LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(timelinesectiondelegatecontrollertest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(tripgroupproxytest.cpp modelverificationpoint.cpp TEST_NAME tripgroupproxytest LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(publictransporttest.cpp TEST_NAME publictransporttest LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(publictransportmatchertest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(timelinedelegatecontrollertest.cpp TEST_NAME timelinedelegatecontrollertest LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(documentmanagertest.cpp TEST_NAME documentmanagertest LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(documentsmodeltest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(statisticstest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(favoritelocationtest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(transfertest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(livedatamanagertest.cpp mocknetworkaccessmanager.cpp TEST_NAME livedatamanagertest LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(healthcertificatemanagertest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(passmanagertest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(calendarhelpertest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(journeysectionmodeltest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(downloadjobtest.cpp mocknetworkaccessmanager.cpp TEST_NAME downloadjobtest LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(onlineticketretrievaltest.cpp mocknetworkaccessmanager.cpp TEST_NAME onlineticketretrievaltest LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(tripgroupmodeltest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(tripgroupcontrollertest.cpp LINK_LIBRARIES Qt::Test itinerary)
ecm_add_test(tripgroupsplitmodeltest.cpp LINK_LIBRARIES Qt::Test itinerary)

ecm_add_test(weathertest.cpp LINK_LIBRARIES Qt::Test itinerary-weather)
target_include_directories(weathertest PRIVATE ${CMAKE_BINARY_DIR})

# verify QML code
add_test(NAME itinerary-self-test COMMAND itinerary-app --self-test)

# Selenium-based UI tests
function(add_selenium_test test)
    if (SeleniumWebDriverATSPI_FOUND OR (CMAKE_SYSTEM_NAME MATCHES "Linux" AND DEFINED ENV{KDECI_BUILD}))
        add_test(NAME appium-${test} COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py)
        set_tests_properties(appium-${test} PROPERTIES
            ENVIRONMENT "USE_CUSTOM_BUS=1;QT_QUICK_CONTROLS_MOBILE=1;QT_QUICK_CONTROLS_STYLE=org.kde.breeze"
            TIMEOUT 120
        )
    endif()
endfunction()

add_selenium_test(apptest)
add_selenium_test(passtest)
add_selenium_test(onlineimporttest)
if (NOT Qt6Core_VERSION VERSION_EQUAL 6.8.0 OR NOT DEFINED ENV{KDECI_BUILD}) # missing https://codereview.qt-project.org/c/qt/qtdeclarative/+/599986
add_selenium_test(reservationtest)
endif()
add_selenium_test(settingstest)
