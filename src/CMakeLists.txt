file(GLOB_RECURSE C2SERVER_CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE C2SERVER_CORE_MODULES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cppm")

add_library(c2server_core STATIC)

target_sources(c2server_core PRIVATE ${C2SERVER_CORE_SOURCES})

target_sources(c2server_core PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES ${C2SERVER_CORE_MODULES})

target_include_directories(c2server_core PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(
   c2server_core
   PRIVATE Boost::beast
           nlohmann_json::nlohmann_json
           OpenSSL::SSL
           OpenSSL::Crypto
           spdlog::spdlog
           Threads::Threads
           c2server_platform_options)
