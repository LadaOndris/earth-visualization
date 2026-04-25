
add_library(project_warnings INTERFACE)
add_library(project_sanitizers INTERFACE)

target_compile_options(project_warnings INTERFACE
    -Wall
    -Wextra
    -Wshadow
    -Wnon-virtual-dtor
    -Wcast-align
    # -Wold-style-cast
    -Woverloaded-virtual
    -Wnull-dereference
    -Wformat=2
    -Wimplicit-fallthrough
    -Wdouble-promotion
    -Wmisleading-indentation
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wuseless-cast
    -Wno-unused-parameter
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
