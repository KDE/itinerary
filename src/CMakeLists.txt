add_definitions(-DTRANSLATION_DOMAIN=\"kde-itinerary\")

add_subdirectory(solidextras)
add_subdirectory(weather)
if (ANDROID)
    add_subdirectory(kandroidextras)
endif()
add_subdirectory(app)
