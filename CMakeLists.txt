cmake_minimum_required(VERSION 3.10)
project(simple_particles)

include(/home/guilherme/vcpkg/scripts/buildsystems/vcpkg.cmake)

include_directories(include)

find_package(MPI REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# Try to find freeglut instead of GLUT
find_package(GLUT REQUIRED)

# Main executable for your particle simulation
add_executable(simple-particles main.cpp)
target_link_libraries(simple-particles
    MPI::MPI_CXX
    Eigen3::Eigen
)

# Executable for rendering the simulation and generating video
add_executable(render-simulation generate_video.cpp)
target_link_libraries(render-simulation
    Eigen3::Eigen
    OpenGL::GL
    GLEW::GLEW
    glfw
    ${GLUT_LIBRARIES}
)

# Ensure GLEW and GLUT are properly linked
if (GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
    target_link_libraries(render-simulation ${GLEW_LIBRARIES})
endif()

if (GLUT_FOUND)
    include_directories(${GLUT_INCLUDE_DIR})
    target_link_libraries(render-simulation ${GLUT_LIBRARIES})
endif()

# Link OpenGL libraries
target_link_libraries(render-simulation ${OPENGL_LIBRARIES})
