set(kpublictransport_srcs
    backends/navitiaclient.cpp
    backends/navitiaparser.cpp

    datatypes/journey.cpp
    datatypes/line.cpp
    datatypes/location.cpp
)
ecm_qt_declare_logging_category(kpublictransport_srcs HEADER logging.h IDENTIFIER KPublicTransport::Log CATEGORY_NAME org.kde.kpublictransport)

add_library(KPublicTransport STATIC ${kpublictransport_srcs})

target_include_directories(KPublicTransport PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>")
target_link_libraries(KPublicTransport
    PUBLIC
        Qt5::Gui
    PRIVATE
        Qt5::Network
)

# ecm_generate_headers(KPublicTransport_FORWARDING_HEADERS
#     HEADER_NAMES
#     PREFIX KPublicTransport
#     REQUIRED_HEADERS KPublicTransport_HEADERS
# )
# ### for testing only
ecm_generate_headers(KPublicTransport_Backends_FORWARDING_HEADERS
    HEADER_NAMES
        NavitiaClient
        NavitiaParser
    PREFIX KPublicTransport
    REQUIRED_HEADERS KPublicTransport_Backends_HEADERS
    RELATIVE backends
)
ecm_generate_headers(KPublicTransport_Datatypes_FORWARDING_HEADERS
    HEADER_NAMES
        Datatypes
        Journey
        Line
        Location
    PREFIX KPublicTransport
    REQUIRED_HEADERS KPublicTransport_Datatypes_HEADERS
    RELATIVE datatypes
)
