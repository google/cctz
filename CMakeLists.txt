# https://github.com/google/oss-policies-info/blob/main/foundational-cxx-support-matrix.md
# As of 2024-07-01, CMake 3.16 is the minimum supported version.
cmake_minimum_required(VERSION 3.16)

project(cctz)

set(CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/cmake
  ${PROJECT_SOURCE_DIR}/cmake/modules
  ${CMAKE_MODULE_PATH})

include(CTest)
include(FeatureSummary)

option(BUILD_TOOLS "Whether or not to build tools" ON)
option(BUILD_EXAMPLES "Whether or not to build examples" ON)
option(BUILD_BENCHMARK "Whether or not to build the benchmark application (has no effect if BUILD_TESTING is disabled)" ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ${BUILD_SHARED_LIBS})

if (BUILD_TESTING)
  if (BUILD_BENCHMARK)
    find_package(benchmark QUIET)
    set_package_properties(benchmark PROPERTIES
      TYPE REQUIRED
      DESCRIPTION "a microbenchmark support library"
      URL "https://github.com/google/benchmark"
    )
  endif()

  find_package(GMock)
  set_package_properties(GMock PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "the Google C++ mocking framework"
    URL "https://github.com/google/googletest"
  )

  find_package(GTest)
  set_package_properties(GTest PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "the Google C++ test framework"
    URL "https://github.com/google/googletest"
  )

  find_package(Threads)
  set_package_properties(Threads PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "the system thread library"
  )
endif()

# If a specific standard is required,
# it can be set from the command line with:
#     cmake -DCMAKE_CXX_STANDARD=[11|14|17|20|23]
function(cctz_target_set_cxx_standard target)
  set(cxx_standard 11)
  target_compile_features(${target} PUBLIC cxx_std_${cxx_standard})
endfunction()

if(APPLE)
  find_library(CoreFoundation CoreFoundation)
endif()

set(CCTZ_HDRS
  include/cctz/time_zone.h
  include/cctz/civil_time_detail.h
  include/cctz/zone_info_source.h
  include/cctz/civil_time.h
  )
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
  src/time_zone_win.cc
  src/time_zone_win.h
  src/tzfile.h
  src/zone_info_source.cc
  $<$<PLATFORM_ID:Windows>:src/time_zone_name_win.cc>
  $<$<PLATFORM_ID:Windows>:src/time_zone_name_win.h>
  $<$<PLATFORM_ID:Windows>:src/time_zone_win_loader.cc>
  $<$<PLATFORM_ID:Windows>:src/time_zone_win_loader.h>
  ${CCTZ_HDRS}
  )
cctz_target_set_cxx_standard(cctz)
target_include_directories(cctz PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )
set_target_properties(cctz PROPERTIES
  PUBLIC_HEADER "${CCTZ_HDRS}"
  )
if(APPLE)
  target_link_libraries(cctz PUBLIC ${CoreFoundation})
endif()
if(WIN32)
  target_link_libraries(cctz PUBLIC advapi32.lib)
endif()
add_library(cctz::cctz ALIAS cctz)

if (BUILD_TOOLS)
  add_executable(time_tool src/time_tool.cc)
  cctz_target_set_cxx_standard(time_tool)
  target_link_libraries(time_tool cctz::cctz)
endif()

if (BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if (BUILD_TESTING)
  add_library(test_time_zone_names
    "src/test_time_zone_names.cc"
    "src/test_time_zone_names.h"
    )

  add_executable(civil_time_test src/civil_time_test.cc)
  cctz_target_set_cxx_standard(civil_time_test)
  target_include_directories(civil_time_test PRIVATE ${GTEST_INCLUDE_DIRS})
  target_link_libraries(civil_time_test
    cctz::cctz
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )
  add_test(civil_time_test civil_time_test)

  add_executable(time_zone_lookup_test src/time_zone_lookup_test.cc)
  cctz_target_set_cxx_standard(time_zone_lookup_test)
  target_include_directories(time_zone_lookup_test PRIVATE ${GTEST_INCLUDE_DIRS})
  target_link_libraries(time_zone_lookup_test
    cctz::cctz
    test_time_zone_names
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )
  add_test(time_zone_lookup_test time_zone_lookup_test)

  add_executable(time_zone_format_test src/time_zone_format_test.cc)
  cctz_target_set_cxx_standard(time_zone_format_test)
  target_link_libraries(time_zone_format_test
    cctz::cctz
    ${CMAKE_THREAD_LIBS_INIT}
    GMock::Main
    )
  add_test(time_zone_format_test time_zone_format_test)

  add_executable(time_zone_win_test src/time_zone_win_test.cc)
  cctz_target_set_cxx_standard(time_zone_win_test)
  target_link_libraries(time_zone_win_test
    cctz::cctz
    ${CMAKE_THREAD_LIBS_INIT}
    GMock::Main
    )
  add_test(time_zone_win_test time_zone_win_test)

  # tests runs on testdata
  set_property(
    TEST
      civil_time_test
      time_zone_format_test
      time_zone_lookup_test
    PROPERTY
      ENVIRONMENT "TZDIR=${CMAKE_CURRENT_SOURCE_DIR}/testdata/zoneinfo"
    )

  if (BUILD_BENCHMARK)
    add_executable(cctz_benchmark src/cctz_benchmark.cc)
    cctz_target_set_cxx_standard(cctz_benchmark)
    target_link_libraries(cctz_benchmark
      cctz::cctz
      test_time_zone_names
      benchmark::benchmark_main
      )
  endif()
endif()

# Install
include(GNUInstallDirs)
install(TARGETS cctz
  EXPORT ${PROJECT_NAME}-targets
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cctz
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
if (BUILD_TOOLS)
  install(TARGETS time_tool
    EXPORT ${PROJECT_NAME}-targets
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
set(CMAKE_INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
install(EXPORT ${PROJECT_NAME}-targets
  NAMESPACE cctz::
  DESTINATION ${CMAKE_INSTALL_CONFIGDIR}
  )
install(FILES cmake/${PROJECT_NAME}-config.cmake
  DESTINATION ${CMAKE_INSTALL_CONFIGDIR}
  )

set(quiet_on_empty QUIET_ON_EMPTY)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES ${quiet_on_empty})
