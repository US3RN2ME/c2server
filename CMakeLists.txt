cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
    "d0edc3af-4c50-42ea-a356-e2862fe7a444"
    CACHE STRING "CMake experimental import std gate")

if(CMAKE_HOST_WIN32)
   set(CMAKE_CXX_FLAGS_INIT "/utf-8")
endif()

project(
   c2server
   VERSION 0.0.1
   LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_MODULE_STD ON)

option(C2SERVER_BUILD_APP "Build the c2server executable" ON)

find_package(Boost REQUIRED COMPONENTS beast)
find_package(spdlog CONFIG REQUIRED)

file(GLOB C2SERVER_CORE_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB C2SERVER_CORE_MODULES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cppm)

add_library(c2server_core STATIC)

target_sources(
   c2server_core
   PRIVATE ${C2SERVER_CORE_SOURCES}
   PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES ${C2SERVER_CORE_MODULES})

target_link_libraries(c2server_core PRIVATE Boost::beast spdlog::spdlog)

if (WIN32)
   target_compile_definitions(c2server_core PRIVATE _WIN32_WINNT=0x0A00 NOMINMAX BOOST_ASIO_DISABLE_WORKING_EXPRESSION_SFINAE)
endif ()

if(C2SERVER_BUILD_APP)
   add_subdirectory(app)
endif()
