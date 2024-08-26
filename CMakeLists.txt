cmake_minimum_required(VERSION 3.10)
project(simple_particles)

include(/home/guilherme/vcpkg/scripts/buildsystems/vcpkg.cmake)

include_directories(include)

find_package(MPI REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(GLUT REQUIRED)
find_package(glm REQUIRED)

# Main executable for your particle simulation
add_executable(simple-particles main.cpp)
target_link_libraries(simple-particles
    MPI::MPI_CXX
    Eigen3::Eigen
)

set_property(TARGET simple-particles PROPERTY CXX_STANDARD 17)

# Executable for rendering the 2d simulation and generating video
add_executable(render-simulation-2d visualize/generate_video_2f.cpp)
target_link_libraries(render-simulation-2d
    Eigen3::Eigen
    OpenGL::GL
    GLEW::GLEW
    glfw
    ${GLUT_LIBRARIES}
)

# Ensure GLEW and GLUT are properly linked
if (GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
    target_link_libraries(render-simulation-2d ${GLEW_LIBRARIES})
endif()

if (GLUT_FOUND)
    include_directories(${GLUT_INCLUDE_DIR})
    target_link_libraries(render-simulation-2d ${GLUT_LIBRARIES})
endif()

if (GLM_FOUND)
    include_directories(${GLM_INCLUDE_DIRS})
    target_link_libraries(render-simulation-2d ${GLM_LIBRARIES})
endif()

# Link OpenGL libraries
target_link_libraries(render-simulation-2d ${OPENGL_LIBRARIES})
