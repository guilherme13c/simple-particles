cmake_minimum_required(VERSION 3.28)

project(SimpleParticles VERSION 0.0.1 LANGUAGES CXX)

include_directories(include)

set(CMAKE_CXX_STANDARD 20)

add_executable(simple-particles
    src/main.cc
)

# openCL
find_package(OpenCL REQUIRED)
target_link_libraries(simple-particles OpenCL::OpenCL)
set(CL_HPP_TARGET_OPENCL_VERSION 300)

# SDL3
find_package(SDL3 REQUIRED)
target_link_libraries(simple-particles SDL3::SDL3)
