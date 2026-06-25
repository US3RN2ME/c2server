cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
    "d0edc3af-4c50-42ea-a356-e2862fe7a444"
    CACHE STRING "CMake experimental import std gate")

if(CMAKE_HOST_WIN32)
   set(CMAKE_CXX_FLAGS_INIT "/utf-8")
endif()

option(C2SERVER_BUILD_TESTS "Build tests" ON)
option(C2SERVER_BUILD_EXAMPLES "Build examples" ON)
option(C2SERVER_BUILD_DOCS "Build API documentation" ON)
option(C2SERVER_INSTALL "Install c2server" ON)

if(C2SERVER_BUILD_TESTS)
   list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

project(
   c2server
   VERSION 0.0.1
   LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

find_package(Boost REQUIRED COMPONENTS beast)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Threads REQUIRED)

include(cmake/PlatformOptions.cmake)

add_subdirectory(src)

if(C2SERVER_INSTALL)
   set(C2SERVER_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/c2server")

   configure_package_config_file(
      "${CMAKE_SOURCE_DIR}/cmake/c2serverConfig.cmake.in" "${PROJECT_BINARY_DIR}/c2serverConfig.cmake"
      INSTALL_DESTINATION "${C2SERVER_INSTALL_CMAKEDIR}")

   write_basic_package_version_file(
      "${PROJECT_BINARY_DIR}/c2serverConfigVersion.cmake"
      VERSION "${PROJECT_VERSION}"
      COMPATIBILITY SameMajorVersion)

   install(
      TARGETS c2server_core c2server_platform_options
      EXPORT c2serverTargets
      ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
      LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
      RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
              FILE_SET cxx_modules
              DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/c2server")

   install(
      EXPORT c2serverTargets
      FILE c2serverTargets.cmake
      NAMESPACE c2server::
      DESTINATION "${C2SERVER_INSTALL_CMAKEDIR}")

   install(FILES "${PROJECT_BINARY_DIR}/c2serverConfig.cmake" "${PROJECT_BINARY_DIR}/c2serverConfigVersion.cmake"
           DESTINATION "${C2SERVER_INSTALL_CMAKEDIR}")

   install(FILES LICENSE README.md DESTINATION "${CMAKE_INSTALL_DATADIR}/c2server")
endif()

if(C2SERVER_BUILD_EXAMPLES)
   add_subdirectory(examples)
endif()

if(C2SERVER_BUILD_TESTS)
   enable_testing()
   add_subdirectory(tests)
endif()

if(C2SERVER_BUILD_DOCS)
   find_package(Doxygen QUIET)
   if(DOXYGEN_FOUND)
      set(DOXYGEN_INPUT_DIR "${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/docs")
      set(DOXYGEN_OUTPUT_DIR "${PROJECT_BINARY_DIR}/docs")
      configure_file("${CMAKE_SOURCE_DIR}/cmake/Doxyfile.in" "${PROJECT_BINARY_DIR}/Doxyfile" @ONLY)

      add_custom_target(
         c2server_docs
         COMMAND ${DOXYGEN_EXECUTABLE} "${PROJECT_BINARY_DIR}/Doxyfile"
         WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
         COMMENT "Generating API documentation with Doxygen"
         VERBATIM)

      if(C2SERVER_INSTALL)
         install(
            DIRECTORY "${DOXYGEN_OUTPUT_DIR}/html/"
            DESTINATION "${CMAKE_INSTALL_DOCDIR}"
            OPTIONAL)
      endif()
   else()
      message(STATUS "Doxygen not found; docs target disabled")
   endif()
endif()
