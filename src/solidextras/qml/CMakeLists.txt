# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(solidextrasqmlplugin solidextrasqmlplugin.cpp)
target_link_libraries(solidextrasqmlplugin
    Qt::Qml
    SolidExtras
)

# make examples work without installation
if (NOT CMAKE_VERSION VERSION_LESS 3.14)
    set_property(TARGET solidextrasqmlplugin PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/org/kde/solidextras)
    file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/org/kde/solidextras/qmldir COPY_ON_ERROR SYMBOLIC)
else()
    message(WARNING "CMake is too old, cannot setup QML plugins to work without installation")
endif()

install(TARGETS solidextrasqmlplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/solidextras)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/solidextras)
