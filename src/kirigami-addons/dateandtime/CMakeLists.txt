# SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
# SPDX-License-Identifier: BSD-2-Clause

if (ANDROID)
    add_library(KF5KirigamiDateAndTime)
    add_library(KF5::KirigamiDateAndTime ALIAS KF5KirigamiDateAndTime)
    target_sources(KF5KirigamiDateAndTime PRIVATE
        lib/androidintegration.cpp
    )
    generate_export_header(KF5KirigamiDateAndTime BASE_NAME KirigamiDateAndTime)
    target_link_libraries(KF5KirigamiDateAndTime PUBLIC Qt${QT_MAJOR_VERSION}::Core)
    if (QT_MAJOR_VERSION EQUAL "5")
        target_link_libraries(KF5KirigamiDateAndTime PUBLIC Qt5::AndroidExtras)
    endif()
    add_subdirectory(android)
    install(TARGETS KF5KirigamiDateAndTime ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

ecm_add_qml_module(itinerary-dateandtimeplugin URI "org.kde.itinerary.kirigamiaddons.dateandtime" VERSION 0.1)

#target_compile_definitions(itinerary-dateandtimeplugin PRIVATE -DTRANSLATION_DOMAIN=\"kirigami_dateandtime\")

target_sources(itinerary-dateandtimeplugin PRIVATE
    lib/yearmodel.cpp
    lib/monthmodel.cpp
    lib/plugin.cpp
    lib/timeinputvalidator.cpp
    lib/qdatetimeparser.cpp
    lib/infinitecalendarviewmodel.cpp
)

ecm_target_qml_sources(itinerary-dateandtimeplugin SOURCES
    ClockFace.qml
    DateInput.qml
    DatePicker.qml
    DatePopup.qml
    TimeLabel.qml
    TimePicker.qml
    TimeInput.qml
)
ecm_target_qml_sources(itinerary-dateandtimeplugin PATH private SOURCES
    private/ClockElement.qml
    private/DesktopDateInput.qml
    private/Hand.qml
    private/MobileDateInput.qml
)

target_link_libraries(itinerary-dateandtimeplugin
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::Qml
    KF${KF_MAJOR_VERSION}::I18n
)
if (TARGET KF5KirigamiDateAndTime)
    target_link_libraries(itinerary-dateandtimeplugin KF5KirigamiDateAndTime)
endif()

ecm_finalize_qml_module(itinerary-dateandtimeplugin DESTINATION ${KDE_INSTALL_QMLDIR})
