project(tests)

set(TEST_TARGET_NAME earth_visualizer_tests)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE SOURCES "*.cpp")
set(INCLUDES ../include)

add_executable(${TEST_TARGET_NAME} ${SOURCES})

target_include_directories(${TEST_TARGET_NAME} PRIVATE
    ${gtest_SOURCE_DIR}/include 
    ${gtest_SOURCE_DIR}
    ${INCLUDES}
)

target_link_libraries(${TEST_TARGET_NAME} earth_visualization_lib)
target_link_libraries(${TEST_TARGET_NAME} gtest gtest_main)

