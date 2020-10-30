# SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

if (ANDROID)
    add_library(KAndroidExtras STATIC
        activity.cpp
        contentresolver.cpp
        context.cpp
        intent.cpp
        javalocale.cpp
        uri.cpp
    )
    target_link_libraries(KAndroidExtras PUBLIC Qt5::AndroidExtras)
else()
    # not for installation, only for tests run on the host
    add_library(KAndroidExtras INTERFACE)
    target_include_directories(KAndroidExtras INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fake)
endif()

target_include_directories(KAndroidExtras INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>")
