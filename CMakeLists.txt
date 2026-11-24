cmake_minimum_required(VERSION 3.25)
project(edgesense VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # .clangd reads build/debug/compile_commands.json

option(EDGESENSE_BUILD_TESTS "Build unit tests" ON)
set(EDGESENSE_SANITIZE "" CACHE STRING "Sanitizer list passed to -fsanitize= (empty = none)")

# Sanitizers apply globally so third-party code (GoogleTest) is instrumented too;
# TSan in particular reports false positives on uninstrumented code.
if(EDGESENSE_SANITIZE)
  add_compile_options(-fsanitize=${EDGESENSE_SANITIZE} -fno-omit-frame-pointer
                      -fno-sanitize-recover=all)
  add_link_options(-fsanitize=${EDGESENSE_SANITIZE})
endif()

# Warnings-as-errors for our own targets only (not for fetched dependencies).
add_library(edgesense_warnings INTERFACE)
target_compile_options(edgesense_warnings INTERFACE -Wall -Wextra -Wpedantic -Wshadow
                                                    -Wconversion -Wsign-conversion -Werror)

# Header-only for now; becomes a regular library once src/ has translation units.
add_library(edgesense INTERFACE)
add_library(edgesense::edgesense ALIAS edgesense)
include(GNUInstallDirs)
target_include_directories(edgesense INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                                               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(edgesense INTERFACE cxx_std_20)

# Install + export so other projects can use find_package(edgesense) and link
# edgesense::edgesense, then #include <edgesense/...>.
include(CMakePackageConfigHelpers)
set(EDGESENSE_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/edgesense)
install(TARGETS edgesense EXPORT edgesenseTargets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT edgesenseTargets NAMESPACE edgesense:: DESTINATION ${EDGESENSE_CMAKE_DIR}
        FILE edgesenseConfig.cmake)
write_basic_package_version_file(${PROJECT_BINARY_DIR}/edgesenseConfigVersion.cmake
                                 COMPATIBILITY SameMajorVersion)
install(FILES ${PROJECT_BINARY_DIR}/edgesenseConfigVersion.cmake DESTINATION ${EDGESENSE_CMAKE_DIR})

if(EDGESENSE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
