#include <Eigen/Dense>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <particle.hh>
#include <thread>
#include <utils/config.hh>
#include <utils/save.hh>
#include <vector>

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Use a constant FOV and adjust only the aspect ratio
    double fov = 45.0;
    double aspectRatio = (double)width / (double)height;
    gluPerspective(fov, aspectRatio, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Adjust camera distance based on height to maintain the same view
    double cameraDistance =
        5.0 * (height / 800.0);         // Adjust based on original height
    gluLookAt(0.0, 0.0, cameraDistance, // Camera position
              0.0, 0.0, 0.0,            // Look-at point
              0.0, 1.0, 0.0);           // Up vector
}

void render_particles(const std::vector<Particle> &particles) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPointSize(3.0f);

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

int main(int argc, char **argv) {
    struct VideoGenerationConfig vcfg;
    parse_video_generation_config(vcfg, argc, argv);

    if (!vcfg.dump_file.is_open()) {
        std::cerr << "Failed to open dump file\n";
        return -1;
    }

    struct SimulationConfig scfg;
    if (!read_simulation_config(vcfg.dump_file, scfg)) {
        std::cerr << "Failed to read simulation configuration\n";
        return -1;
    }

    std::vector<Particle> particles(scfg.particle_count);
    if (!read_simulation_state(vcfg.dump_file, particles)) {
        std::cerr << "Failed to read simulation data\n";
        return -1;
    }

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(scfg.world_width, scfg.world_height,
                                          "Simple Particles", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    framebuffer_size_callback(window, scfg.world_width, scfg.world_height);

    while (!glfwWindowShouldClose(window)) {
        render_particles(particles);

        if (!read_simulation_state(vcfg.dump_file, particles))
            break;

        std::this_thread::sleep_for(
            std::chrono::milliseconds(static_cast<int>(scfg.time_step * 1000)));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    vcfg.dump_file.close();
    glfwTerminate();
    return 0;
}
