add_executable(c2server)

target_sources(c2server PRIVATE main.cpp)

target_link_libraries(c2server PRIVATE c2server_core)

configure_file("${CMAKE_SOURCE_DIR}/config/config.json" "${CMAKE_BINARY_DIR}/config.json" COPYONLY)
