cmake_minimum_required(VERSION 2.8.12)

project(cctz)

set(CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/cmake
  ${PROJECT_SOURCE_DIR}/cmake/modules
  ${CMAKE_MODULE_PATH})

include(CheckCXXCompilerFlag)
include(CTest)
include(GNUInstallDirs)

option(BUILD_EXAMPLES "Whether or not to build examples" ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall)
endif()

# if a specific standard is required, this can be set from the command line:
#     cmake -DCMAKE_CXX_STANDARD=[11|14|17]
if (NOT CMAKE_CXX_STANDARD)
  # C++11 is the minimum version required
  set(CMAKE_CXX_STANDARD 11)
endif()

# CMAKE_CXX_STANDARD is only supported since CMake >= 3.1
if (CMAKE_VERSION VERSION_LESS "3.1")
  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-std=c++${CMAKE_CXX_STANDARD})
  endif ()
endif()

add_library(cctz
  src/civil_time_detail.cc
  src/time_zone_fixed.cc
  src/time_zone_fixed.h
  src/time_zone_format.cc
  src/time_zone_if.cc
  src/time_zone_if.h
  src/time_zone_impl.cc
  src/time_zone_impl.h
  src/time_zone_info.cc
  src/time_zone_info.h
  src/time_zone_libc.cc
  src/time_zone_libc.h
  src/time_zone_lookup.cc
  src/time_zone_posix.cc
  src/time_zone_posix.h
  src/tzfile.h
  src/zone_info_source.cc
  )
add_library(cctz::cctz ALIAS cctz)

target_include_directories(cctz PUBLIC
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )

set(CMAKE_CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

install(DIRECTORY include/cctz DESTINATION include)
install(TARGETS cctz
        EXPORT ${PROJECT_NAME}-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(EXPORT ${PROJECT_NAME}-targets
        NAMESPACE cctz::
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})
install(FILES cmake/${PROJECT_NAME}-config.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})

# not (yet) supported on Windows because of getopt.h
if (NOT MSVC)
  add_executable(time_tool src/time_tool.cc)
  target_link_libraries(time_tool cctz::cctz)
endif()

if (BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if (BUILD_TESTING)
  find_package(benchmark REQUIRED)
  find_package(GMock REQUIRED)
  find_package(GTest REQUIRED)
  find_package(Threads REQUIRED)

  add_executable(civil_time_test src/civil_time_test.cc)
  target_include_directories(civil_time_test PRIVATE ${GTEST_INCLUDE_DIRS})
  target_link_libraries(civil_time_test
    cctz::cctz
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )
  add_test(civil_time_test civil_time_test)

  add_executable(time_zone_lookup_test src/time_zone_lookup_test.cc)
  target_include_directories(time_zone_lookup_test PRIVATE ${GTEST_INCLUDE_DIRS})
  target_link_libraries(time_zone_lookup_test
    cctz::cctz
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )
  add_test(time_zone_lookup_test time_zone_lookup_test)

  add_executable(time_zone_format_test src/time_zone_format_test.cc)
  target_link_libraries(time_zone_format_test
    cctz::cctz
    ${CMAKE_THREAD_LIBS_INIT}
    GMock::Main
    )
  add_test(time_zone_format_test time_zone_format_test)

  # tests runs on testdata
  set_property(
    TEST
      civil_time_test
      time_zone_format_test
      time_zone_lookup_test
    PROPERTY
      ENVIRONMENT "TZDIR=${CMAKE_CURRENT_SOURCE_DIR}/testdata/zoneinfo"
    )

  add_executable(benchmarks src/benchmarks.cc)
  target_link_libraries(benchmarks cctz::cctz benchmark::benchmark)
endif()
