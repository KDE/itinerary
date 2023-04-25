# SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>
# SPDX-License-Identifier: BSD-3-Clause

kcoreaddons_add_plugin(itinerarythumbnail SOURCES itinerarycreator.cpp INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/thumbcreator")

target_link_libraries(itinerarythumbnail
    KF${KF_MAJOR_VERSION}::KIOGui
    Qt::Gui
    KPim${KF_MAJOR_VERSION}::PkPass
)
