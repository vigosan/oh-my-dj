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

add_executable(
  ohmydj-preset-tests
  test_presets.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/stream-presets.cpp
)
target_include_directories(ohmydj-preset-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
add_test(NAME presets COMMAND ohmydj-preset-tests)

add_executable(
  ohmydj-health-tests
  test_health.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/stream-health.cpp
)
target_include_directories(ohmydj-health-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
add_test(NAME health COMMAND ohmydj-health-tests)

add_executable(
  ohmydj-plan-tests
  test_plan.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/rotation-plan.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/duration.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/rotation-step.cpp
)
target_include_directories(ohmydj-plan-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
add_test(NAME plan COMMAND ohmydj-plan-tests)
