# SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

set(kandroidextras_srcs
    android/activity.cpp
    android/android_headers.cpp
    android/contentresolver.cpp
    android/context.cpp
    android/intent.cpp
    android/uri.cpp

    java/java_headers.cpp
    java/javalocale.cpp

    jni/jni_headers.cpp
)

if (NOT ANDROID)
    list (APPEND kandroidextras_srcs fake/mock_impl.cpp)
endif()

add_library(KAndroidExtras STATIC ${kandroidextras_srcs})
generate_export_header(KAndroidExtras BASE_NAME KAndroidExtras)
target_link_libraries(KAndroidExtras PUBLIC Qt5::Core)

if (ANDROID)
    target_link_libraries(KAndroidExtras PUBLIC Qt5::AndroidExtras)
else()
    target_include_directories(KAndroidExtras PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/fake)
endif()

ecm_generate_headers(KAndroidExtras_android_FORWARDING_HEADERS
    HEADER_NAMES
        Activity
        AndroidTypes
        ContentResolver
        Context
        Intent
        ManifestPermission
        OpenableColumns
        Settings
        Uri
    PREFIX KAndroidExtras
    REQUIRED_HEADERS KAndroidExtras_android_HEADERS
    RELATIVE android
)

ecm_generate_headers(KAndroidExtras_java_FORWARDING_HEADERS
    HEADER_NAMES
        JavaLocale
        JavaTypes
    PREFIX KAndroidExtras
    REQUIRED_HEADERS KAndroidExtras_java_HEADERS
    RELATIVE java
)

ecm_generate_headers(KAndroidExtras_jni_FORWARDING_HEADERS
    HEADER_NAMES
        JniArray
        JniProperty
        JniSignature
        JniTypes
        JniTypeTraits
    PREFIX KAndroidExtras
    REQUIRED_HEADERS KAndroidExtras_jni_HEADERS
    RELATIVE jni
)
