cmake_minimum_required(VERSION 3.27)
project(earth_visualization)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)

include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(include)

file(GLOB_RECURSE SOURCES_C "src/*.c")
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")
add_executable(earth_visualization main.cpp ${SOURCES} ${SOURCES_C} ${HEADERS})
target_link_libraries(earth_visualization ${OPENGL_gl_LIBRARY} glfw glm imgui)
