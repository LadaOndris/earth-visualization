project(tests)

add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})


file(GLOB_RECURSE SOURCES "*.cpp")
add_executable(tests_run ${SOURCES})

target_link_libraries(tests_run earth_visualization_lib)
target_link_libraries(tests_run gtest gtest_main)

