cmake_minimum_required(VERSION 2.8.12)

include(ExternalProject)

set_property(DIRECTORY PROPERTY EP_BASE .)

if (NOT CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cxx11-toolchain.cmake")
endif()

ExternalProject_Add(ep_benchmark
  URL https://github.com/google/benchmark/archive/master.zip
  CMAKE_ARGS
      -DBENCHMARK_ENABLE_TESTING=OFF
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
      -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>)
ExternalProject_Get_Property(ep_benchmark install_dir)
set(ep_benchmark_install_dir "${install_dir}")

ExternalProject_Add(ep_gmock
  URL https://github.com/google/googletest/archive/master.zip
  CMAKE_ARGS
      -DBUILD_GMOCK=ON
      -Dgmock_build_tests=OFF
      -Dgtest_build_samples=OFF
      -Dgtest_build_tests=OFF
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
      -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>)
ExternalProject_Get_Property(ep_gmock install_dir)
set(ep_gmock_install_dir "${install_dir}")

ExternalProject_Add(ep_cctz
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/.."
  LIST_SEPARATOR ::
  CMAKE_ARGS
      -DBUILD_EXAMPLES=ON
      -DBUILD_TESTING=ON
      "-DCMAKE_PREFIX_PATH=${ep_benchmark_install_dir}::${ep_gmock_install_dir}"
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
  INSTALL_COMMAND ""
  TEST_COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure
  BUILD_ALWAYS 1
  DEPENDS ep_benchmark ep_gmock)
