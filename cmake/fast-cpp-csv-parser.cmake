
include(FetchContent)

FetchContent_Declare(
    fast-cpp-csv-parser
    GIT_REPOSITORY https://github.com/ben-strasser/fast-cpp-csv-parser.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(fast-cpp-csv-parser)

set(CSV_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
file(MAKE_DIRECTORY ${CSV_INCLUDE_DIR}/fast-cpp-csv-parser)
file(COPY ${fast-cpp-csv-parser_SOURCE_DIR}/csv.h
     DESTINATION ${CSV_INCLUDE_DIR}/fast-cpp-csv-parser)

add_library(fast-cpp-csv-parser INTERFACE)
target_include_directories(fast-cpp-csv-parser INTERFACE ${CSV_INCLUDE_DIR})
