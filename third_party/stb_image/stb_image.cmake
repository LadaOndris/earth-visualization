
add_library(stb_image STATIC
    ${CMAKE_CURRENT_LIST_DIR}/stb_image.cpp
)
target_include_directories(stb_image SYSTEM PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/
)
