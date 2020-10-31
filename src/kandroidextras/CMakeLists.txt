# SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

set(kandroidextras_srcs
    java/javalocale.cpp
)

if (ANDROID)
    list (APPEND kandroidextras_srcs
        android/activity.cpp
        android/contentresolver.cpp
        android/context.cpp
        android/intent.cpp
        android/uri.cpp
    )
endif()

add_library(KAndroidExtras STATIC ${kandroidextras_srcs})
target_link_libraries(KAndroidExtras PUBLIC Qt5::Core)

if (ANDROID)
    target_link_libraries(KAndroidExtras PUBLIC Qt5::AndroidExtras)
else()
    target_include_directories(KAndroidExtras PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/fake)
endif()

ecm_generate_headers(KAndroidExtras_android_FORWARDING_HEADERS
    HEADER_NAMES
        Activity
        ContentResolver
        Context
        Intent
        ManifestPermission
        Settings
        Uri
    PREFIX KAndroidExtras
    REQUIRED_HEADERS KAndroidExtras_android_HEADERS
    RELATIVE android
)

ecm_generate_headers(KAndroidExtras_java_FORWARDING_HEADERS
    HEADER_NAMES
        JavaLocale
    PREFIX KAndroidExtras
    REQUIRED_HEADERS KAndroidExtras_java_HEADERS
    RELATIVE java
)

ecm_generate_headers(KAndroidExtras_jni_FORWARDING_HEADERS
    HEADER_NAMES
        JniProperty
        JniSignature
        JniTypes
    PREFIX KAndroidExtras
    REQUIRED_HEADERS KAndroidExtras_jni_HEADERS
    RELATIVE jni
)
