cmake_minimum_required(VERSION 3.27)
project(earth_visualization)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

include_directories(${OPENGL_INCLUDE_DIR})
include_directories(include)

add_executable(earth_visualization main.cpp src/glad.c src/stb_image.cpp src/ellipsoid.h src/ellipsoid.cpp src/tesselation/SubdivisionSurfaces.h src/tesselation/SubdivisionSurfaces.cpp)
target_link_libraries(earth_visualization ${OPENGL_gl_LIBRARY} glfw glm)
