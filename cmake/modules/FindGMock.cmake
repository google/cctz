# Find the GMock libraries
#
# This module defines the following variable:
# GMOCK_FOUND - true if GMock has been found and can be used
#
# This module defines the following IMPORTED targets:
# GMock::GMock
#       The Google Mock gmock library, if found;
#       the GTest libraries are automatically added
# GMock::Main
#       The Google Mock gmock_main library, if found

find_path(GMOCK_INCLUDE_DIR gmock/gmock.h)
find_library(GMOCK_LIBRARY gmock)
find_library(GMOCK_MAIN_LIBRARY gmock_main)

find_package(GTest)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMock DEFAULT_MSG
  GMOCK_LIBRARY GMOCK_MAIN_LIBRARY
  GMOCK_INCLUDE_DIR
  GTEST_FOUND
)
mark_as_advanced(GMOCK_INCLUDE_DIR GMOCK_LIBRARY GMOCK_MAIN_LIBRARY)

if (GMOCK_FOUND AND NOT TARGET GMock::GMock)
  add_library(GMock::GMock UNKNOWN IMPORTED)
  set_target_properties(GMock::GMock PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GTEST_INCLUDE_DIRS};${GMOCK_INCLUDE_DIR}")
  set_target_properties(GMock::GMock PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
    IMPORTED_LOCATION "${GMOCK_LIBRARY}"
    INTERFACE_LINK_LIBRARIES "${GTEST_LIBRARIES}")

  add_library(GMock::Main UNKNOWN IMPORTED)
  set_target_properties(GMock::Main PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
    IMPORTED_LOCATION "${GMOCK_MAIN_LIBRARY}"
    INTERFACE_LINK_LIBRARIES GMock::GMock)
endif()
