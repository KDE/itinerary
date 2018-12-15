set(kpublictransport_srcs
    departurereply.cpp
    departurerequest.cpp
    journeyreply.cpp
    journeyrequest.cpp
    manager.cpp
    reply.cpp

    backends/abstractbackend.cpp
    backends/hafasmgatebackend.cpp
    backends/navitiabackend.cpp
    backends/navitiaparser.cpp

    datatypes/departure.cpp
    datatypes/journey.cpp
    datatypes/line.cpp
    datatypes/location.cpp

    networks/networks.qrc
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

ecm_generate_headers(KPublicTransport_FORWARDING_HEADERS
    HEADER_NAMES
        DepartureReply
        DepartureRequest
        JourneyReply
        JourneyRequest
        Manager
        Reply
    PREFIX KPublicTransport
    REQUIRED_HEADERS KPublicTransport_HEADERS
)
# # ### for testing only
ecm_generate_headers(KPublicTransport_Backends_FORWARDING_HEADERS
    HEADER_NAMES
        NavitiaParser
    PREFIX KPublicTransport
    REQUIRED_HEADERS KPublicTransport_Backends_HEADERS
    RELATIVE backends
)
ecm_generate_headers(KPublicTransport_Datatypes_FORWARDING_HEADERS
    HEADER_NAMES
        Datatypes
        Departure
        Journey
        Line
        Location
    PREFIX KPublicTransport
    REQUIRED_HEADERS KPublicTransport_Datatypes_HEADERS
    RELATIVE datatypes
)

install(FILES org_kde_kpublictransport.categories DESTINATION ${KDE_INSTALL_CONFDIR})
