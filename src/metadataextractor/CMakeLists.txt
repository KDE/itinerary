# SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>
# SPDX-License-Identifier: BSD-3-Clause

add_library(kfilemetadata_itineraryextractor MODULE itineraryextractor.cpp)

target_link_libraries(kfilemetadata_itineraryextractor
    KF6::FileMetaData
    KF6::I18n
    Qt::Gui
    KPim6::Itinerary
    KPim6::PkPass
)

set_target_properties(kfilemetadata_itineraryextractor PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata")
install(TARGETS kfilemetadata_itineraryextractor
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata)
