#include <Eigen/Dense>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

// Particle structure (simplified for rendering)
struct Particle {
    Eigen::Vector3d position;
    Eigen::Vector3d velocity;
    double mass;
};

// Function to read the next state from the binary dump file
bool read_simulation_state(std::ifstream &file,
                           std::vector<Particle> &particles) {
    for (auto &particle : particles) {
        file.read(reinterpret_cast<char *>(particle.position.data()),
                  sizeof(double) * 3);
        file.read(reinterpret_cast<char *>(particle.velocity.data()),
                  sizeof(double) * 3);
        file.read(reinterpret_cast<char *>(&particle.mass), sizeof(double));
        if (file.eof())
            return false;
    }
    return true;
}

// Function to render the particles
void render_particles(const std::vector<Particle> &particles) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the point size
    glPointSize(5.0f); // Adjust the size as needed

    // Enable smooth point rendering
    glEnable(GL_POINT_SMOOTH);

    // Set point color (e.g., white)
    glColor3f(1.0f, 1.0f, 1.0f);

    // Render each particle as a point
    glBegin(GL_POINTS);
    for (const auto &particle : particles) {
        glVertex3d(particle.position.x(), particle.position.y(),
                   particle.position.z());
    }
    glEnd();

    // Disable smooth points (optional, depends on your use case)
    glDisable(GL_POINT_SMOOTH);
}

int main() {
    // Initialize GLFW and create a window
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

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    // Open the binary dump file
    std::ifstream dump_file("dump/simulation.bin", std::ios::binary);
    if (!dump_file.is_open()) {
        std::cerr << "Failed to open dump file\n";
        return -1;
    }

    // Initialize particles (update with correct number of particles)
    std::vector<Particle> particles(2); // Adjust size based on your simulation
    if (!read_simulation_state(dump_file, particles)) {
        std::cerr << "Failed to read simulation data\n";
        return -1;
    }

    // Rendering loop
    while (!glfwWindowShouldClose(window)) {
        render_particles(particles);

        // Read the next state
        if (!read_simulation_state(dump_file, particles))
            break;

        // Sleep for 0.01 seconds
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    dump_file.close();
    glfwTerminate();
    return 0;
}
