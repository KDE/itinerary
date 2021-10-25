# SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>
# SPDX-License-Identifier: BSD-3-Clause

set(itinerarythumbnail_SRCS itinerarycreator.cpp)
add_library(itinerarythumbnail MODULE ${itinerarythumbnail_SRCS})
target_link_libraries(itinerarythumbnail
    KF5::KIOWidgets
    Qt5::Gui
    KPim::PkPass
)

install(TARGETS itinerarythumbnail DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES itinerarythumbnail.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
