add_executable(c2server)
c2server_configure_target(c2server)

target_sources(c2server PRIVATE main.cpp)

target_link_libraries(c2server PRIVATE c2server_core)

configure_file("${CMAKE_SOURCE_DIR}/config/config.json" "${CMAKE_CURRENT_BINARY_DIR}/config.json" COPYONLY)
