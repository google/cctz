cmake_minimum_required(VERSION 2.8.12)

project(cctz)

set(CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/cmake
  ${PROJECT_SOURCE_DIR}/cmake/modules
  ${CMAKE_MODULE_PATH})

include(CTest)
include(FeatureSummary)

option(BUILD_EXAMPLES "Whether or not to build examples" ON)

if (BUILD_TESTING)
  find_package(benchmark)
  set_package_properties(benchmark PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "a microbenchmark support library"
    URL "https://github.com/google/benchmark"
  )

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

# Starting from CMake >= 3.1, if a specific standard is required,
# it can be set from the command line with:
#     cmake -DCMAKE_CXX_STANDARD=[11|14|17]
function(cctz_target_set_cxx_standard target)
  set(cxx_standard 11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
      target_compile_options(${target} PRIVATE -std=c++${cxx_standard})
    endif()
  elseif (CMAKE_VERSION VERSION_LESS "3.8")
    set_property(TARGET ${target} PROPERTY CXX_STANDARD ${cxx_standard})
  else()
    target_compile_features(${target} PUBLIC cxx_std_${cxx_standard})
  endif()
endfunction()

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
cctz_target_set_cxx_standard(cctz)
add_library(cctz::cctz ALIAS cctz)

target_include_directories(cctz PUBLIC include)

add_executable(time_tool src/time_tool.cc)
cctz_target_set_cxx_standard(time_tool)
target_link_libraries(time_tool cctz::cctz)

if (BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if (BUILD_TESTING)
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
  cctz_target_set_cxx_standard(benchmarks)
  target_link_libraries(benchmarks cctz::cctz benchmark::benchmark)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
