file(GLOB_RECURSE C2SERVER_CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE C2SERVER_CORE_MODULES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cppm")
set(C2SERVER_CORE_PRIVATE_MODULES ${C2SERVER_CORE_MODULES})
list(FILTER C2SERVER_CORE_MODULES EXCLUDE REGEX "/server/detail/")
list(FILTER C2SERVER_CORE_PRIVATE_MODULES INCLUDE REGEX "/server/detail/")

configure_file("${CMAKE_SOURCE_DIR}/cmake/version.cppm.in" "${PROJECT_BINARY_DIR}/generated/version.cppm" @ONLY)
list(APPEND C2SERVER_CORE_MODULES "${PROJECT_BINARY_DIR}/generated/version.cppm")

add_library(c2server_core STATIC)
add_library(c2server::core ALIAS c2server_core)
set_target_properties(c2server_core PROPERTIES EXPORT_NAME core)

target_sources(c2server_core PRIVATE ${C2SERVER_CORE_SOURCES})

target_sources(
   c2server_core
   PUBLIC FILE_SET
          cxx_modules
          TYPE
          CXX_MODULES
          BASE_DIRS
          "${CMAKE_CURRENT_SOURCE_DIR}"
          "${PROJECT_BINARY_DIR}/generated"
          FILES
          ${C2SERVER_CORE_MODULES}
   PRIVATE FILE_SET
           private_cxx_modules
           TYPE
           CXX_MODULES
           BASE_DIRS
           "${CMAKE_CURRENT_SOURCE_DIR}"
           FILES
           ${C2SERVER_CORE_PRIVATE_MODULES})

target_compile_features(c2server_core PUBLIC cxx_std_23)

target_include_directories(c2server_core PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

target_link_libraries(
   c2server_core
   PUBLIC c2server_platform_options
   PRIVATE Boost::beast nlohmann_json::nlohmann_json OpenSSL::SSL OpenSSL::Crypto spdlog::spdlog Threads::Threads)
