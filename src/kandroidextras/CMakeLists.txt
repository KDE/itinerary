add_library(KAndroidExtras STATIC
    activity.cpp
    contentresolver.cpp
    context.cpp
    intent.cpp
    locale.cpp
    uri.cpp
)
target_include_directories(KAndroidExtras INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>")
target_link_libraries(KAndroidExtras PRIVATE Qt5::AndroidExtras)
