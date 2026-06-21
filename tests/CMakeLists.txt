# Standalone test project: builds the pure rotation logic with no OBS/Qt
# dependencies, so `ctest` runs anywhere.
#
#   cmake -S tests -B tests/build
#   cmake --build tests/build
#   ctest --test-dir tests/build --output-on-failure

cmake_minimum_required(VERSION 3.16)
project(oh-my-dj-tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_executable(
  ohmydj-tests
  test_rotation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/duration.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/rotation-step.cpp
)
target_include_directories(ohmydj-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_test(NAME rotation COMMAND ohmydj-tests)
