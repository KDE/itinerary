# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.0)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "21")
set (RELEASE_SERVICE_VERSION_MINOR "04")
set (RELEASE_SERVICE_VERSION_MICRO "1")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(itinerary VERSION ${RELEASE_SERVICE_VERSION})

set(KF5_MIN_VERSION 5.64)
if (ANDROID)
    set(KF5_MIN_VERSION 5.76) # for FindGradle
endif()
find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_SOURCE_DIR}/cmake)
if (POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()

include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(ECMGenerateHeaders)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(GenerateExportHeader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

ecm_setup_version(PROJECT VARIABLE_PREFIX ITINERARY VERSION_HEADER itinerary_version.h)
set(ITINERARY_DETAILED_VERSION_STRING "${ITINERARY_VERSION}")
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
    find_package(Git)
    set_package_properties(Git PROPERTIES TYPE OPTIONAL PURPOSE "Determine exact build version.")
    if (GIT_FOUND)
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE _git_rev)
        string(REGEX REPLACE "\n" "" _git_rev "${_git_rev}")
        set(ITINERARY_DETAILED_VERSION_STRING "${ITINERARY_VERSION} (${_git_rev})")
    endif()
endif()

set(QT_MIN_VERSION 5.14) # for new style QML Connections
if (ANDROID)
    set(QT_MIN_VERSION 5.15.1) # for content: support
endif()

# build-time dependencies
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS Test Quick Positioning Location QuickControls2)
find_package(Qt5 CONFIG QUIET OPTIONAL_COMPONENTS QuickCompiler)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS I18n CoreAddons Contacts Notifications)
find_package(KF5 ${KF5_MIN_VERSION} OPTIONAL_COMPONENTS Holidays NetworkManagerQt)
find_package(KPimPkPass CONFIG REQUIRED)
find_package(KPimItinerary 5.14.41 CONFIG REQUIRED)
find_package(KPublicTransport CONFIG REQUIRED)
find_package(KOSMIndoorMap CONFIG REQUIRED)
find_package(SharedMimeInfo 1.0 REQUIRED)
find_package(ZLIB REQUIRED)
set_package_properties("ZLIB" PROPERTIES PURPOSE "Needed for retrieving weather forecast data.")
set_package_properties(KF5Solid PROPERTIES TYPE OPTIONAL TYPE RUNTIME PURPOSE "Used for controlling the screen brightness.")

include(ECMQMLModules)
ecm_find_qmlmodule(QtLocation 5.11)
ecm_find_qmlmodule(QtPositioning 5.11)
ecm_find_qmlmodule(org.kde.prison 1.0)
ecm_find_qmlmodule(org.kde.kosmindoormap 1.0)
if (NOT ANDROID)
    ecm_find_qmlmodule(Qt.labs.platform 1.0)
endif()

# runtime dependencies are build-time dependencies on Android
if (ANDROID)
    find_package(Qt5 REQUIRED COMPONENTS AndroidExtras Svg)
    find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Archive Kirigami2 Prison)
    if (NOT DEFINED BREEZEICONS_DIR AND EXISTS ${CMAKE_SOURCE_DIR}/../breeze-icons)
        set(BREEZEICONS_DIR ${CMAKE_SOURCE_DIR}/../breeze-icons)
    endif()
    find_package(OpenSSL REQUIRED)
    find_package(Gradle REQUIRED)
else()
    find_package(Qt5 REQUIRED COMPONENTS Widgets DBus)
    find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS DBusAddons)
    find_package(KF5 ${KF5_MIN_VERSION} COMPONENTS Solid)
endif()

add_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)
add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050d00)
if (ANDROID)
    set(CMAKE_MODULE_EXE_FLAGS "-Wl,--fatal-warnings -Wl,--no-undefined ${CMAKE_EXE_LINKER_FLAGS}")
endif()

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

if (NOT ANDROID)
    install(FILES org_kde_itinerary.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
endif()
ki18n_install(po)
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
