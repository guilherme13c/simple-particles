#pragma once

#include <cassert>
#include <chrono>
#include <cinttypes>
#include <fstream>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

#ifdef GRAPHICS
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif

typedef struct {
    std::vector<float> x;
    std::vector<float> y;
} spatial_property_2f;

class World2f {
  private:
    bool should_run;

    float max_x, min_x, max_y, min_y, max_vel, damping_factor, dt;
    spatial_property_2f positions, velocities;
    uint64_t N, duration;

    std::ofstream dump_file;

    void kernel(const size_t p1, const size_t p2) {
        const float dx = positions.x[p2] - positions.x[p1],
                    dy = positions.y[p2] - positions.y[p1];
        const float dist2 = dx * dx + dy * dy;

        if (dist2 < 0.01f || dist2 > 10000.0)
            return;

        const float force = 1.0f / dist2;

        velocities.x[p1] += force * dx;
        velocities.y[p1] += force * dy;
        velocities.x[p2] -= force * dx;
        velocities.y[p2] -= force * dy;
    }

    void create_random_particles(void) {
        positions.x.reserve(N);
        positions.y.reserve(N);
        velocities.x.reserve(N);
        velocities.y.reserve(N);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> pos_dist_x(min_x, max_x);
        std::uniform_real_distribution<> pos_dist_y(min_y, max_y);
        std::uniform_real_distribution<> vel_dist(-max_vel, max_vel);

        for (uint64_t i = 0; i < N; i++) {
            positions.x.push_back(pos_dist_x(gen));
            positions.y.push_back(pos_dist_y(gen));
            velocities.x.push_back(vel_dist(gen));
            velocities.y.push_back(vel_dist(gen));
        }
    }

    void save_state(void) {
        for (uint64_t i = 0; i < N; i++) {
            dump_file.write(reinterpret_cast<const char *>(&positions.x[i]),
                            sizeof(float));
            dump_file.write(reinterpret_cast<const char *>(&positions.y[i]),
                            sizeof(float));
            dump_file.write(reinterpret_cast<const char *>(&velocities.x[i]),
                            sizeof(float));
            dump_file.write(reinterpret_cast<const char *>(&velocities.y[i]),
                            sizeof(float));
        }
    }

#ifdef GRAPHICS
    void render_particles(void) {
        if (!glfwInit()) {
            return;
        }

        float world_width = max_x - min_x;
        float world_height = max_y - min_y;
        float aspect_ratio = world_width / world_height;

        int window_width = 1200;
        int window_height = static_cast<int>(window_width / aspect_ratio);
        if (aspect_ratio > 1.0f) {
            window_height = 1200;
            window_width = static_cast<int>(window_height * aspect_ratio);
        }

        GLFWwindow *window = glfwCreateWindow(window_width, window_height,
                                              "Simple Particles", NULL, NULL);
        if (!window) {
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);
        glewInit();

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(min_x, max_x, min_y, max_y, -1, 1);

        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT);

            glBegin(GL_POINTS);
            for (uint64_t i = 0; i < N; i++) {
                glVertex2f(positions.x[i], positions.y[i]);
            }
            glEnd();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glfwDestroyWindow(window);
        glfwTerminate();

        should_run = false;
    }
#endif

  public:
    World2f(void)
        : N(0), max_x(0), min_x(0), max_y(0), min_y(0), max_vel(0), dt(0.1),
          duration(10), damping_factor(0),
          dump_file("dumpfile.bin", std::ios::binary), should_run(false) {}

    World2f(uint64_t N, float max_x, float min_x, float max_y, float min_y,
            float max_vel, float dt, uint64_t duration)
        : N(N), max_x(max_x), min_x(min_x), max_y(max_y), min_y(min_y),
          max_vel(max_vel), dt(dt), duration(duration), damping_factor(0),
          dump_file("dumpfile.bin", std::ios::binary), should_run(false) {
        create_random_particles();
    }

    ~World2f() { dump_file.close(); }

    void set_damping_factor(const float xi) { damping_factor = xi; }

    void update_particles(void) {
        for (uint64_t i = 0; i < N; ++i) {
            for (uint64_t j = i + 1; j < N; ++j) {
                kernel(i, j);
            }
        }

        for (uint64_t i = 0; i < N; ++i) {
            velocities.x[i] *= (1 - damping_factor * this->dt);
            velocities.y[i] *= (1 - damping_factor * this->dt);

            positions.x[i] += velocities.x[i] * this->dt;
            positions.y[i] += velocities.y[i] * this->dt;

            if (positions.x[i] < min_x) {
                positions.x[i] = min_x;
                velocities.x[i] *= -1;
            }
            if (positions.x[i] > max_x) {
                positions.x[i] = max_x;
                velocities.x[i] *= -1;
            }
            if (positions.y[i] < min_y) {
                positions.y[i] = min_y;
                velocities.y[i] *= -1;
            }
            if (positions.y[i] > max_y) {
                positions.y[i] = max_y;
                velocities.y[i] *= -1;
            }
        }

        save_state();

        std::this_thread::sleep_for(
            std::chrono::milliseconds(static_cast<int>(dt * 1000)));
    }

    void run(void) {
        should_run = true;

#ifdef GRAPHICS
        std::thread render_thread(&World2f::render_particles, this);
#endif

        while (should_run) {
            update_particles();
        }

#ifdef GRAPHICS
        render_thread.join();
#endif
    }
};
