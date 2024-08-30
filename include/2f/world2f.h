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

#include <CL/opencl.hpp>
#include <mpi.h>

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

    cl::Context context;
    cl::CommandQueue queue;
    cl::Program program;
    cl::Kernel kernel;

    int mpi_rank, mpi_size;

    MPI_Comm mpi_comm;
    uint64_t local_N;

    void partition_particles(void);

    void exchange_boundary_data(void);

    void init_opencl(void);

    void run_opencl_kernel(void);

    void create_random_particles(void);

    void update_positions(void);

#ifdef GRAPHICS
    void render_particles(void);
#endif

    void compute(void);

  public:
    World2f(void);

    World2f(uint64_t N, float max_x, float min_x, float max_y, float min_y,
            float max_vel, float dt);

    ~World2f(void);

    void set_damping_factor(const float xi);

    void run(void);
};
