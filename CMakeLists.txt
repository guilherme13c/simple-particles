cmake_minimum_required(VERSION 3.10)
project(simple_particles)

include(/home/guilherme/vcpkg/scripts/buildsystems/vcpkg.cmake)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 REQUIRED)

add_executable(simple_particles src/main.cpp src/world2f.cpp)

include_directories(include)

target_compile_definitions(simple_particles PRIVATE GRAPHICS)

target_link_libraries(simple_particles
    OpenGL::GL
    GLEW::GLEW
    glfw
)

set_property(TARGET simple_particles PROPERTY CXX_STANDARD 17)
