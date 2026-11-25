include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.18.0
  GIT_SHALLOW TRUE
  EXCLUDE_FROM_ALL)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# Every tests/*_test.cpp is compiled into one test binary.
file(GLOB EDGESENSE_TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp)

if(NOT EDGESENSE_TEST_SOURCES)
  message(STATUS "edgesense: no tests/*_test.cpp yet, skipping test target")
  return()
endif()

add_executable(edgesense_tests ${EDGESENSE_TEST_SOURCES})
target_link_libraries(edgesense_tests PRIVATE edgesense::edgesense edgesense_warnings
                                              GTest::gtest_main)
gtest_discover_tests(edgesense_tests DISCOVERY_MODE PRE_TEST)
