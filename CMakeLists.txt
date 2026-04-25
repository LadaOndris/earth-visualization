cmake_minimum_required(VERSION 3.27)

set(TARGET_NAME earth_visualization)
set(LIB_TARGET_NAME ${TARGET_NAME}_lib)

project(${TARGET_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/compile_options.cmake)
include(cmake/imgui.cmake)
include(cmake/fast-cpp-csv-parser.cmake)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Freetype REQUIRED)

file(GLOB_RECURSE SOURCES_C "src/*.c")
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

add_executable(${TARGET_NAME} main.cpp ${SOURCES} ${SOURCES_C} ${HEADERS} third_party/glad.c)

target_include_directories(${TARGET_NAME} PRIVATE
    ${GLFW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
    include/
)
target_include_directories(${TARGET_NAME} SYSTEM PRIVATE
    third_party/
)

target_link_libraries(${TARGET_NAME}
    OpenGL::GL 
    glfw 
    glm::glm 
    imgui 
    Freetype::Freetype
    fast-cpp-csv-parser
    project_warnings
    project_sanitizers
)

add_library(${LIB_TARGET_NAME} ${SOURCES} ${SOURCES_C} ${HEADERS} third_party/glad.c)

target_include_directories(${LIB_TARGET_NAME} PRIVATE
    ${GLFW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
    include/
)
target_include_directories(${LIB_TARGET_NAME} SYSTEM PRIVATE
    third_party/
)

target_link_libraries(${LIB_TARGET_NAME}
    OpenGL::GL 
    glfw 
    glm::glm 
    imgui 
    Freetype::Freetype
    fast-cpp-csv-parser
)

add_subdirectory(tests)
