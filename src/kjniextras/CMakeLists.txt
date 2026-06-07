# SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

if(NOT ANDROID)
    return()
endif()

add_executable(kjnitest kjnitest.cpp)
target_link_libraries(kjnitest PRIVATE Qt6::Core)
