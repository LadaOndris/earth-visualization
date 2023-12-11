cmake_minimum_required(VERSION 3.27)
project(earth_visualization)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(Freetype REQUIRED)

include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${FREETYPE_INCLUDE_DIRS})
include_directories(include)

file(GLOB_RECURSE SOURCES_C "src/*.c")
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# Executable
add_executable(earth_visualization main.cpp ${SOURCES} ${SOURCES_C} ${HEADERS} src/WorldCitiesReader.cpp)
target_link_libraries(earth_visualization
        ${OPENGL_gl_LIBRARY} glfw glm imgui
        ${FREETYPE_LIBRARIES})

# Library for tests
add_library(earth_visualization_lib ${SOURCES} ${SOURCES_C} ${HEADERS})
target_link_libraries(earth_visualization_lib
        ${OPENGL_gl_LIBRARY} glfw glm imgui
        ${FREETYPE_LIBRARIES})

add_subdirectory(tests)