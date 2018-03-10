include(KDECompilerSettings NO_POLICY_SCOPE)

set(pkpass_srcs
    barcode.cpp
    boardingpass.cpp
    field.cpp
    location.cpp
    pass.cpp
)
ecm_qt_declare_logging_category(pkpass_srcs
    HEADER logging.h
    IDENTIFIER KPkPass::Log
    CATEGORY_NAME org.kde.pkpass
)

add_library(KPkPass SHARED ${pkpass_srcs})
generate_export_header(KPkPass BASE_NAME KPkPass)
set_target_properties(KPkPass PROPERTIES
    VERSION ${ITINERARY_VERSION_STRING}
    SOVERSION ${ITINERARY_SOVERSION}
    EXPORT_NAME KPkPass
)
target_link_libraries(KPkPass
    PUBLIC
        Qt5::Gui
    PRIVATE
        KF5::Archive
)

ecm_generate_headers(KPkPass_HEADERS
    HEADER_NAMES
        Barcode
        BoardingPass
        Field
        Location
        Pass
    PREFIX KPkPass
    REQUIRED_HEADERS KPkPass_HEADERS
)

install(TARGETS KPkPass EXPORT KPkPassTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${KPkPass_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kpkpass_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KPkPass
)

if (NOT ANDROID)
    install(FILES application-vnd-apple-pkpass.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
    update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
endif()
