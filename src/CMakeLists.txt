add_executable(callsign2name main.cpp)

find_package(Boost REQUIRED COMPONENTS program_options)
target_link_libraries(callsign2name PRIVATE Boost::boost Boost::program_options)

find_package(cpr CONFIG REQUIRED)
IF(WIN32)
    target_link_libraries(callsign2name PRIVATE cpr::cpr)
ELSE()
    target_link_libraries(callsign2name PRIVATE cpr)
ENDIF()

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(callsign2name PRIVATE nlohmann_json::nlohmann_json)

find_package(pugixml CONFIG REQUIRED)
target_link_libraries(callsign2name PRIVATE pugixml::pugixml)