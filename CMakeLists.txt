cmake_minimum_required(VERSION 3.10)
project(simple_particles)

set(CMAKE_CXX_COMPILER "mpicxx")

include(/home/guilherme/vcpkg/scripts/buildsystems/vcpkg.cmake)
include_directories(include /home/guilherme/vcpkg/packages/openmpi_x64-linux/include/)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(OpenCL 3.0 REQUIRED)
find_package(MPI REQUIRED)

add_executable(simple_particles src/main.cpp src/world2f.cpp)

target_compile_definitions(simple_particles PRIVATE GRAPHICS)

target_link_libraries(simple_particles
    OpenGL::GL
    GLEW::GLEW
    glfw
    OpenCL::OpenCL
    MPI::MPI_CXX
)

add_definitions(-DCL_HPP_TARGET_OPENCL_VERSION=300)

set_property(TARGET simple_particles PROPERTY CXX_STANDARD 17)
