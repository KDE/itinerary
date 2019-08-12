add_library(KAndroidExtras STATIC
    contentresolver.cpp
)
target_include_directories(KAndroidExtras INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>")
target_link_libraries(KAndroidExtras PRIVATE Qt5::AndroidExtras)
