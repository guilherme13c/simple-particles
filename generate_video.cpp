#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <particle.h>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void read_configuration(std::ifstream &dump_file, float &dt, float &max_x,
                        float &min_x, float &max_y, float &min_y, float &max_z,
                        float &min_z, uint64_t &N) {
    if (!dump_file.is_open()) {
        throw std::runtime_error("Configuration file is not open.");
    }

    // Read configuration from the file
    dump_file.read(reinterpret_cast<char *>(&N), sizeof(N));
    dump_file.read(reinterpret_cast<char *>(&dt), sizeof(dt));
    dump_file.read(reinterpret_cast<char *>(&max_x), sizeof(max_x));
    dump_file.read(reinterpret_cast<char *>(&min_x), sizeof(min_x));
    dump_file.read(reinterpret_cast<char *>(&max_y), sizeof(max_y));
    dump_file.read(reinterpret_cast<char *>(&min_y), sizeof(min_y));
    dump_file.read(reinterpret_cast<char *>(&max_z), sizeof(max_z));
    dump_file.read(reinterpret_cast<char *>(&min_z), sizeof(min_z));

    if (!dump_file) {
        throw std::runtime_error("Error reading configuration data.");
    }
}

bool read_state_from_file(std::ifstream &dump_file, uint64_t N,
                          std::vector<Particle3f> &particles) {
    if (!dump_file.is_open()) {
        throw std::runtime_error("State file is not open.");
    }

    if (particles.size() < N) {
        throw std::runtime_error("Particle vector is not allocated properly.");
    }

    for (uint64_t i = 0; i < N; ++i) {
        Eigen::Vector3f position, velocity;

        if (dump_file.eof()) {
            return false; // File has ended
        }

        dump_file.read(reinterpret_cast<char *>(position.data()),
                       sizeof(Eigen::Vector3f));
        dump_file.read(reinterpret_cast<char *>(velocity.data()),
                       sizeof(Eigen::Vector3f));

        particles[i].get_position() = position;
        particles[i].get_velocity() = velocity;
    }

    return true; // Data successfully read, file has not ended
}

void draw_particles(std::vector<Particle3f> &particles) {
    glBegin(GL_POINTS);
    for (Particle3f &particle : particles) {
        const Eigen::Vector3f position = particle.get_position();
        float x = position.x();
        float y = position.y();
        float z = position.z();

        float scale =
            std::max(0.1f, 1.0f / (z + 1.0f)); // avoid division by zero
        glPointSize(10.0f); // adjust the size multiplier as needed

        std::cout << scale << std::endl;

        glVertex3f(x, y, z);
    }
    glEnd();
}

void save_frame(const std::string &filename, int width, int height) {
    std::vector<unsigned char> buffer(width * height * 3);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());

    std::vector<unsigned char> flipped_buffer(width * height * 3);
    for (int y = 0; y < height; ++y) {
        std::memcpy(flipped_buffer.data() + (height - y - 1) * width * 3,
                    buffer.data() + y * width * 3, width * 3);
    }

    stbi_write_png(filename.c_str(), width, height, 3, flipped_buffer.data(),
                   width * 3);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <input dump file> <output video file>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string input_filename = argv[1];
    std::string output_filename = argv[2];

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return EXIT_FAILURE;
    }

    GLFWwindow *window =
        glfwCreateWindow(800, 600, "Particle Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glewInit();

    float dt, max_x, min_x, max_y, min_y, max_z, min_z;
    uint64_t num_particles;

    std::ifstream dump_file(input_filename, std::ios::binary);
    if (!dump_file.is_open()) {
        std::cerr << "Failed to open input dump file" << std::endl;
        return EXIT_FAILURE;
    }

    read_configuration(dump_file, dt, max_x, min_x, max_y, min_y, max_z, min_z,
                       num_particles);
    std::vector<Particle3f> particles(num_particles);

    int width = 800;
    int height = 600;
    int frame_count = 0;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (read_state_from_file(dump_file, num_particles, particles)) {
            draw_particles(particles);
        } else {
            break;
        }

        std::string frame_filename =
            "frame/frame_" + std::to_string(frame_count++) + ".png";
        save_frame(frame_filename, width, height);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::string command = "ffmpeg -r " + std::to_string(std::ceil(1 / dt)) +
                          " -i frame/frame_%d.png -vcodec libx264 -crf "
                          "25 -pix_fmt yuv420p -y " +
                          output_filename;
    system(command.c_str());

    system("rm frame/*");

    return EXIT_SUCCESS;
}
