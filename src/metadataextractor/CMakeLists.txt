# SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>
# SPDX-License-Identifier: BSD-3-Clause

add_library(kfilemetadata_itineraryextractor MODULE itineraryextractor.cpp)

target_link_libraries(kfilemetadata_itineraryextractor
    KF${KF_MAJOR_VERSION}::FileMetaData
    KF${KF_MAJOR_VERSION}::I18n
    Qt::Gui
    KPim${KF_MAJOR_VERSION}::Itinerary
    KPim${KF_MAJOR_VERSION}::PkPass
)

set_target_properties(kfilemetadata_itineraryextractor PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf${QT_MAJOR_VERSION}/kfilemetadata")
install(TARGETS kfilemetadata_itineraryextractor
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf${QT_MAJOR_VERSION}/kfilemetadata)
