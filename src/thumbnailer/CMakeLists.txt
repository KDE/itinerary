# SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>
# SPDX-License-Identifier: BSD-3-Clause

kcoreaddons_add_plugin(itinerarythumbnail SOURCES itinerarycreator.cpp INSTALL_NAMESPACE "kf6/thumbcreator")

target_link_libraries(itinerarythumbnail
    KF6::KIOGui
    Qt::Gui
    KPim6::PkPass
)
