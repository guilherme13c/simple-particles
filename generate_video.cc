#include <Eigen/Dense>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <particle.hh>
#include <thread>
#include <utils/save.hh>
#include <vector>

void render_particles(const std::vector<Particle> &particles) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPointSize(5.0f);

    glEnable(GL_POINT_SMOOTH);

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_POINTS);
    for (const auto &particle : particles) {
        glVertex3d(particle.position.x(), particle.position.y(),
                   particle.position.z());
    }
    glEnd();

    glDisable(GL_POINT_SMOOTH);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow *window =
        glfwCreateWindow(1200, 800, "Particle Simulation", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    std::ifstream dump_file("dump/simulation.dump", std::ios::binary);
    if (!dump_file.is_open()) {
        std::cerr << "Failed to open dump file\n";
        return -1;
    }

    std::vector<Particle> particles(2);
    if (!read_simulation_state(dump_file, particles)) {
        std::cerr << "Failed to read simulation data\n";
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {
        render_particles(particles);

        if (!read_simulation_state(dump_file, particles))
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    dump_file.close();
    glfwTerminate();
    return 0;
}
