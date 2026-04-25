
include(FetchContent)

FetchContent_Declare(
    fast-cpp-csv-parser
    GIT_REPOSITORY https://github.com/ben-strasser/fast-cpp-csv-parser.git
    GIT_TAG        master
    SOURCE_DIR     ${CMAKE_BINARY_DIR}/include/fast-cpp-csv-parser
)
FetchContent_MakeAvailable(fast-cpp-csv-parser)

add_library(fast-cpp-csv-parser INTERFACE)
target_include_directories(fast-cpp-csv-parser SYSTEM INTERFACE ${CMAKE_BINARY_DIR}/include)
