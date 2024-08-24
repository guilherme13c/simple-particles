#include <3f.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Shader source code for 3D
const char *vertex_shader_source = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos, 1.0);
            gl_PointSize = 5.0;
        }
    )";

const char *fragment_shader_source = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        }
    )";

GLuint compile_shader(const char *shader_source, GLenum shader_type) {
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, nullptr, info_log);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n"
                  << info_log << std::endl;
        throw std::runtime_error("Shader compilation failed");
    }

    return shader;
}

GLuint create_shader_program(const char *vertex_shader_source,
                             const char *fragment_shader_source) {
    GLuint vertex_shader =
        compile_shader(vertex_shader_source, GL_VERTEX_SHADER);
    GLuint fragment_shader =
        compile_shader(fragment_shader_source, GL_FRAGMENT_SHADER);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    GLint success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(shader_program, 512, nullptr, info_log);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << info_log << std::endl;
        throw std::runtime_error("Shader program linking failed");
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

void read_configuration(std::ifstream &dump_file, float &dt, float &max_x,
                        float &min_x, float &max_y, float &min_y, float &max_z,
                        float &min_z, uint64_t &N) {
    if (!dump_file.is_open()) {
        throw std::runtime_error("Configuration file is not open.");
    }

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
            return false;
        }

        dump_file.read(reinterpret_cast<char *>(position.data()),
                       sizeof(Eigen::Vector3f));
        dump_file.read(reinterpret_cast<char *>(velocity.data()),
                       sizeof(Eigen::Vector3f));

        particles[i].get_position() = position;
        particles[i].get_velocity() = velocity;
    }

    return true;
}

void create_particle_vertex_array(GLuint &vao, GLuint &vbo,
                                  std::vector<Particle3f> &particles) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(Eigen::Vector3f),
                 nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void update_particle_vertex_array(GLuint vbo,
                                  std::vector<Particle3f> &particles) {
    std::vector<Eigen::Vector3f> positions(particles.size());
    for (size_t i = 0; i < particles.size(); ++i) {
        positions[i] = particles[i].get_position();
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    positions.size() * sizeof(Eigen::Vector3f),
                    positions.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_particles(GLuint vao, uint64_t num_particles, GLuint shader_program) {
    glUseProgram(shader_program);
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(num_particles));
    glBindVertexArray(0);
    glUseProgram(0);
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

    GLuint shader_program =
        create_shader_program(vertex_shader_source, fragment_shader_source);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-10.0, 10.0, -7.5, 7.5, -10.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

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

    GLuint vao, vbo;
    create_particle_vertex_array(vao, vbo, particles);

    int width = 800;
    int height = 600;
    int frame_count = 0;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (read_state_from_file(dump_file, num_particles, particles)) {
            update_particle_vertex_array(vbo, particles);
            draw_particles(vao, num_particles, shader_program);

            std::string frame_filename =
                "frame/frame_" + std::to_string(frame_count++) + ".png";
            save_frame(frame_filename, width, height);

            glfwSwapBuffers(window);
            glfwPollEvents();
        } else {
            break;
        }
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shader_program);

    glfwDestroyWindow(window);
    glfwTerminate();

    std::string command = "ffmpeg -r " + std::to_string(std::ceil(1 / dt)) +
                          " -i frame/frame_%d.png -vcodec libx264 -crf "
                          "25 -pix_fmt yuv420p -y " +
                          output_filename;
    system(command.c_str());

    system("rm frame/*");

    return 0;
}
