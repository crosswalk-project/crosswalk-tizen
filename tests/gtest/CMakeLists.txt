# Defines pre_project_set_up_hermetic_build() and set_up_hermetic_build().
include(cmake/hermetic_build.cmake OPTIONAL)

if (COMMAND pre_project_set_up_hermetic_build)
  pre_project_set_up_hermetic_build()
endif()

# SET(TARGET_TESTS_GTEST "gtest") in root CMakeList.txt
project(gtest CXX C)

if (COMMAND set_up_hermetic_build)
  set_up_hermetic_build()
endif()

# Define helper functions and macros used by Google Test.
include(cmake/internal_utils.cmake)

config_compiler_and_linker()  # Defined in internal_utils.cmake.

# Where Google Test's .h files can be found.
include_directories(
  ${gtest_SOURCE_DIR}/include
  ${gtest_SOURCE_DIR})

# Where Google Test's libraries can be found.
link_directories(${gtest_BINARY_DIR}/src)

cxx_library(${TESTS_GTEST} "${cxx_strict}" src/gtest-all.cc)
cxx_library(${TESTS_GTEST_MAIN} "${cxx_strict}" src/gtest_main.cc)
target_link_libraries(${TESTS_GTEST_MAIN} ${TESTS_GTEST})
