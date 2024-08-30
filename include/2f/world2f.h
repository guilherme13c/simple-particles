#pragma once

#include <cassert>
#include <chrono>
#include <cinttypes>
#include <fstream>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

#include <2f/structs.h>

#ifdef GRAPHICS
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif

class World2f {
  private:
    bool should_run;

    float max_x, min_x, max_y, min_y, max_vel, damping_factor, dt;
    spatial_property_2f positions, velocities;
    uint64_t N;

    uint64_t grid_size_x, grid_size_y;
    float subdomain_width, subdomain_height;
    std::vector<std::vector<std::vector<size_t>>> subdomains;

    void kernel(const size_t p1, const size_t p2);

    void create_random_particles(void);

    void update_subdomain_particles(uint64_t grid_x, uint64_t grid_y);

    void update_positions(void);

    void update_particles(void);

#ifdef GRAPHICS
    void render_particles(void);
#endif

    void compute(void);

    void assign_particles_to_subdomains(void);

  public:
    World2f(void);

    World2f(uint64_t N, float max_x, float min_x, float max_y, float min_y,
            float max_vel, float dt, uint64_t duration,
            uint64_t grid_size_x = 1, uint64_t grid_size_y = 1);

    void set_damping_factor(const float xi);

    void run(void);
};
