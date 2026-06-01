cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
    "d0edc3af-4c50-42ea-a356-e2862fe7a444"
    CACHE STRING "CMake experimental import std gate")

if(CMAKE_HOST_WIN32)
   set(CMAKE_CXX_FLAGS_INIT "/utf-8")
endif()

option(C2SERVER_BUILD_TESTS "Build tests" ON)
option(C2SERVER_BUILD_APP "Build the c2server executable" ON)

if(C2SERVER_BUILD_TESTS)
   list(APPEND VCPKG_MANIFEST_FEATURES "tests")
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

find_package(Boost REQUIRED COMPONENTS beast)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Threads REQUIRED)

include(cmake/PlatformOptions.cmake)

add_subdirectory(src)

if(C2SERVER_BUILD_APP)
   add_subdirectory(app)
endif()

if(C2SERVER_BUILD_TESTS)
   enable_testing()
   add_subdirectory(tests)
endif()
