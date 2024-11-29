# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "25")
set (RELEASE_SERVICE_VERSION_MINOR "03")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(itinerary VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION 6.5)
set(KF_MIN_VERSION 6.8)
set(GEAR_MIN_VERSION 24.08)
set(PIM_MIN_VERSION 6.2)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(ECMGenerateHeaders)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(GenerateExportHeader)
include(ECMQmlModule)
include(ECMDeprecationSettings)
include(ECMFeatureSummary)
include(ECMAddAndroidApk)

set(CMAKE_CXX_STANDARD 20)
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

# build-time dependencies
find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Test Quick Positioning QuickControls2)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS I18n CoreAddons Contacts Notifications CalendarCore Holidays UnitConversion)
find_package(KF6 ${KF_MIN_VERSION} OPTIONAL_COMPONENTS NetworkManagerQt)
find_package(KF6KirigamiAddons 1.6.0 REQUIRED)
find_package(KPim6PkPass ${PIM_MIN_VERSION} CONFIG REQUIRED)
find_package(KPim6Itinerary ${PIM_MIN_VERSION} CONFIG REQUIRED)
find_package(KPublicTransport ${GEAR_MIN_VERSION} CONFIG REQUIRED)
find_package(KOSMIndoorMap ${GEAR_MIN_VERSION} CONFIG REQUIRED)
find_package(KHealthCertificate ${GEAR_MIN_VERSION} CONFIG)
find_package(Qt6Keychain CONFIG REQUIRED)
find_package(LibIcal 3.0)
find_package(ZLIB REQUIRED)
set(QUOTIENT_FORCE_NAMESPACED_INCLUDES ON)
find_package(QuotientQt6 0.9.0 CONFIG)
find_package(SeleniumWebDriverATSPI)

set_package_properties("ZLIB" PROPERTIES PURPOSE "Needed for retrieving weather forecast data.")
set_package_properties(LibIcal PROPERTIES PURPOSE "Needed for Android calendar integration.")
set_package_properties(KF6Solid PROPERTIES TYPE RUNTIME PURPOSE "Used for controlling the screen brightness.")
set_package_properties(KHealthCertificate PROPERTIES TYPE OPTIONAL PURPOSE "Needed for the vaccination certificate manager.")
set_package_properties(QuotientQt6 PROPERTIES TYPE OPTIONAL PURPOSE "Needed for Matrix location sharing.")
set_package_properties(SeleniumWebDriverATSPI PROPERTIES PURPOSE "Needed for GUI tests" URL "https://invent.kde.org/sdk/selenium-webdriver-at-spi" TYPE OPTIONAL)

include(ECMFindQmlModule)
# TODO all of these should be required, but that doesn't work with Flatpak/Android yet...
ecm_find_qmlmodule(QtLocation 6.0)
ecm_find_qmlmodule(QtMultimedia 6.0)
ecm_find_qmlmodule(QtPositioning 6.0)
ecm_find_qmlmodule(org.kde.kitemmodels ${KF_MIN_VERSION})
ecm_find_qmlmodule(org.kde.prison ${KF_MIN_VERSION})
ecm_find_qmlmodule(org.kde.prison.scanner ${KF_MIN_VERSION})
ecm_find_qmlmodule(org.kde.kosmindoormap ${GEAR_MIN_VERSION} REQUIRED)
ecm_find_qmlmodule(org.kde.kopeninghours ${GEAR_MIN_VERSION} REQUIRED)

# runtime dependencies are build-time dependencies on Android
if (ANDROID)
    find_package(Qt6 REQUIRED COMPONENTS Svg)
    find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Archive ColorScheme Kirigami Prison)
    if (NOT DEFINED BREEZEICONS_DIR AND EXISTS ${CMAKE_SOURCE_DIR}/../breeze-icons)
        set(BREEZEICONS_DIR ${CMAKE_SOURCE_DIR}/../breeze-icons)
    endif()
    find_package(OpenSSL REQUIRED)
    find_package(Gradle REQUIRED)
    set_package_properties(LibIcal PROPERTIES TYPE REQUIRED)
else()
    find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)
    find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS DBusAddons QQC2DesktopStyle WindowSystem)
    find_package(KF6 ${KF_MIN_VERSION} OPTIONAL_COMPONENTS Solid Crash FileMetaData KIO)
    find_package(SharedMimeInfo 1.0 REQUIRED)
    set(HAVE_DBUS ON)
endif()
if (TARGET KF6::Crash)
    set(HAVE_KCRASH TRUE)
endif()
if (TARGET KHealthCertificate)
    set(HAVE_KHEALTHCERTIFICATE TRUE)
endif()
if (TARGET QuotientQt6)
    set(HAVE_MATRIX TRUE)
else()
    set(HAVE_MATRIX FALSE)
endif()

ecm_set_disabled_deprecation_versions(
    QT 6.4.0
    KF 6.5.0
)

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

# install Fontconfig workaround for emoji fonts in Flatpaks
if (FLATPAK_BUILD)
    install(FILES flatpak/99-noto-color-emoji.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/fonts/local.conf)
endif()

ecm_qt_install_logging_categories(
    EXPORT ITINERARY
    FILE org_kde_itinerary.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
ki18n_install(po)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
