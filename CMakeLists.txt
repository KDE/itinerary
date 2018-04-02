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
find_package(KPimPkPass REQUIRED)
find_package(KPimItinerary REQUIRED)
find_package(QmlLint)
set_package_properties(QmlLint PROPERTIES URL "https://qt.io" PURPOSE "Validate QML code.")
find_package(SharedMimeInfo 1.0 REQUIRED)

include(ECMQMLModules)
ecm_find_qmlmodule(org.kde.prison 1.0)

# runtime dependencies are build-time dependencies on Android
if (ANDROID)
    find_package(Qt5 REQUIRED COMPONENTS AndroidExtras Svg)
    find_package(KF5 REQUIRED COMPONENTS Archive Kirigami2 Prison)
    find_package(GammaRay)
    if (NOT DEFINED BREEZEICONS_DIR AND EXISTS ${CMAKE_SOURCE_DIR}/../breeze-icons)
        set(BREEZEICONS_DIR ${CMAKE_SOURCE_DIR}/../breeze-icons)
    endif()
endif()

add_subdirectory(src)
add_subdirectory(autotests)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
