function(c2server_add_example TARGET SOURCE)
   add_executable(${TARGET})
   c2server_configure_target(${TARGET})
   target_sources(${TARGET} PRIVATE ${SOURCE})
   target_link_libraries(${TARGET} PRIVATE c2server_core)
endfunction()

c2server_add_example(c2server_payload_server payload_server.cpp)
c2server_add_example(c2server_swagger_server swagger_server.cpp)
c2server_add_example(c2server_hello_world hello_world.cpp)
c2server_add_example(c2server_middleware_server middleware_server.cpp)

configure_file("${CMAKE_SOURCE_DIR}/config/config.json" "${CMAKE_CURRENT_BINARY_DIR}/config.json" COPYONLY)
