
add_library(project_warnings INTERFACE)
add_library(project_sanitizers INTERFACE)

target_compile_options(project_warnings INTERFACE
    -Wall
    -Wextra
    -Wshadow
    -Wnon-virtual-dtor
    -Wcast-align
    -Wunused
    -Woverloaded-virtual
    -Wnull-dereference
    -Wformat=2
    -Wimplicit-fallthrough
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
