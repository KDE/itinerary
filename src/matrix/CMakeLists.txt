# SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-matrix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-matrix.h)

add_library(KMatrix STATIC)

ecm_add_qml_module(KMatrix
    GENERATE_PLUGIN_SOURCE
    URI "org.kde.quotient"
    DEPENDENCIES "QtCore" "QtQml"
    SOURCES
        types.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/config-matrix.h
)

generate_export_header(KMatrix BASE_NAME KMatrix)

if (HAVE_MATRIX)
    target_sources(KMatrix PRIVATE
        matrixbeacon.cpp
        matrixmanager.cpp
        matrixroomsmodel.cpp
        matrixroomssortproxymodel.cpp
    )

    target_link_libraries(KMatrix PUBLIC Qt::Core QuotientQt6 Qt::Qml PRIVATE KF6::I18n)

    ecm_generate_headers(KMatrix_FORWARDING_HEADERS
        HEADER_NAMES
            MatrixManager
            MatrixRoomsModel
        PREFIX KMatrix
        REQUIRED_HEADERS KMatrix_HEADERS
    )
endif()
