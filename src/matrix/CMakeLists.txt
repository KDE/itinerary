# SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(KMatrix STATIC)
ecm_add_qml_module(KMatrix
    GENERATE_PLUGIN_SOURCE
    URI "org.kde.itinerary.matrix"
    DEPENDENCIES "QtCore"
)

if (HAVE_MATRIX)
    target_sources(KMatrix PRIVATE
        matrixbeacon.cpp
        matrixmanager.cpp
        matrixroomsmodel.cpp
        matrixroomssortproxymodel.cpp
        types.h
    )

    target_link_libraries(KMatrix PUBLIC QuotientQt6 PRIVATE KF6::I18n)

    ecm_generate_headers(KMatrix_FORWARDING_HEADERS
        HEADER_NAMES
            MatrixManager
            MatrixRoomsModel
        PREFIX KMatrix
        REQUIRED_HEADERS KMatrix_HEADERS
    )
else()
    target_sources(KMatrix PRIVATE
        matrixbeaconstub.cpp
    )
endif()
