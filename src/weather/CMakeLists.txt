set(weather_srcs
    weatherforecast.cpp
    weatherforecastmanager.cpp
)

add_library(itinerary-weather STATIC ${weather_srcs})
target_link_libraries(itinerary-weather
    PUBLIC
        Qt5::Core
    PRIVATE
        Qt5::Network
        ${ZLIB_LIBRARIES}
)
