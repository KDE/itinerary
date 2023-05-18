# SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(KMatrix STATIC)

target_sources(KMatrix PRIVATE
        matrixbeacon.cpp
        matrixmanager.cpp
        matrixroomsmodel.cpp
        matrixroomssortproxymodel.cpp
)

generate_export_header(KMatrix BASE_NAME KMatrix)
target_link_libraries(KMatrix PUBLIC Qt::Core Quotient PRIVATE KF${KF_MAJOR_VERSION}::I18n)

ecm_generate_headers(KMatrix_FORWARDING_HEADERS
        HEADER_NAMES
        MatrixManager
        MatrixRoomsModel
        PREFIX KMatrix
        REQUIRED_HEADERS KMatrix_HEADERS
        )
