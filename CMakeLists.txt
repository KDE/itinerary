cmake_minimum_required(VERSION 3.0)
project(itinerary VERSION 0.0.1)

find_package(ECM 5.38 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_SOURCE_DIR}/cmake)

include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(GenerateExportHeader)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
ecm_setup_version(PROJECT VARIABLE_PREFIX ITINERARY VERSION_HEADER itinerary_version.h)

# build-time dependencies
find_package(Qt5 REQUIRED COMPONENTS Test Quick)
find_package(KF5 REQUIRED COMPONENTS I18n)
find_package(KF5Contacts CONFIG REQUIRED)
find_package(KPimPkPass CONFIG REQUIRED)
find_package(KPimItinerary CONFIG REQUIRED)
find_package(QmlLint)
set_package_properties(QmlLint PROPERTIES URL "https://qt.io" PURPOSE "Validate QML code.")
find_package(SharedMimeInfo 1.0 REQUIRED)
find_package(ZLIB REQUIRED)
set_package_properties("ZLIB" PROPERTIES PURPOSE "Needed for retrieving weather forecast data.")

include(ECMQMLModules)
ecm_find_qmlmodule(org.kde.prison 1.0)
ecm_find_qmlmodule(Qt.labs.platform 1.0)

# runtime dependencies are build-time dependencies on Android
if (ANDROID)
    find_package(Qt5 REQUIRED COMPONENTS AndroidExtras Svg)
    find_package(KF5 REQUIRED COMPONENTS Archive Kirigami2 Prison)
    if (NOT DEFINED BREEZEICONS_DIR AND EXISTS ${CMAKE_SOURCE_DIR}/../breeze-icons)
        set(BREEZEICONS_DIR ${CMAKE_SOURCE_DIR}/../breeze-icons)
    endif()
    find_package(OpenSSL REQUIRED)
else()
    find_package(Qt5 REQUIRED COMPONENTS Widgets Positioning)
    find_package(KF5 REQUIRED COMPONENTS DBusAddons)
endif()

add_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)
add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

add_subdirectory(src)
add_subdirectory(autotests)

install(FILES org_kde_itinerary.categories DESTINATION ${KDE_INSTALL_CONFDIR})
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
