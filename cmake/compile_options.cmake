
add_library(project_warnings INTERFACE)
add_library(project_sanitizers INTERFACE)

target_compile_options(project_warnings INTERFACE
    -Wall
    -Wextra
    -Wshadow
    -Wcast-align
    -Wnull-dereference
    -Wformat=2
    -Wimplicit-fallthrough
    -Wdouble-promotion
    -Wmisleading-indentation
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wno-unused-parameter
    $<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>
    $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>
    $<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>
    $<$<COMPILE_LANGUAGE:CXX>:-Wuseless-cast>
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(project_sanitizers INTERFACE
        -fsanitize=address,undefined,leak
        -fno-omit-frame-pointer
    )
    target_link_options(project_sanitizers INTERFACE
        -fsanitize=address,undefined,leak
    )
endif()
