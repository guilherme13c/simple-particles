#include <2f/world2f.h>

void World2f::create_random_particles(void) {
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

#ifdef GRAPHICS
void World2f::render_particles(void) {
    if (!glfwInit()) {
        return;
    }

    float world_width = max_x - min_x;
    float world_height = max_y - min_y;
    float aspect_ratio = world_width / world_height;

    int window_width = 900;
    int window_height = static_cast<int>(window_width / aspect_ratio);
    if (aspect_ratio > 1.0f) {
        window_height = 900;
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

void World2f::compute(void) {
    init_opencl();
    while (should_run) {
        run_opencl_kernel();
        update_positions();
    }
}

World2f::World2f(void)
    : N(0), max_x(0), min_x(0), max_y(0), min_y(0), max_vel(0), dt(0.1),
      damping_factor(0), should_run(false) {
    MPI_Init(nullptr, nullptr);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    mpi_comm = MPI_COMM_WORLD;
}

World2f::World2f(uint64_t N, float max_x, float min_x, float max_y, float min_y,
                 float max_vel, float dt)
    : N(N), max_x(max_x), min_x(min_x), max_y(max_y), min_y(min_y),
      max_vel(max_vel), dt(dt), damping_factor(0), should_run(false) {
    MPI_Init(nullptr, nullptr);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    mpi_comm = MPI_COMM_WORLD;

    create_random_particles();
}

World2f::~World2f(void) { MPI_Finalize(); }

void World2f::set_damping_factor(const float xi) { damping_factor = xi; }

void World2f::update_positions(void) {
    std::vector<std::thread> threads;

    const size_t num_threads = std::thread::hardware_concurrency();
    const size_t particles_per_thread = N / num_threads;

    for (size_t t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            size_t start = t * particles_per_thread;
            size_t end =
                (t == num_threads - 1) ? N : start + particles_per_thread;

            for (size_t i = start; i < end; ++i) {
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
        });
    }

    for (auto &thread : threads) {
        thread.join();
    }
}

void World2f::run(void) {
    should_run = true;

    std::cout << "Starting compute thread\n";
    std::thread compute_thread(&World2f::compute, this);

    if (mpi_rank == 0) {
#ifdef GRAPHICS
        std::cout << "Starting rendering thread\n";
        std::thread render_thread(&World2f::render_particles, this);
        render_thread.join();
#endif
    }

    while (true) {
        MPI_Bcast(&should_run, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

        if (!should_run) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    compute_thread.join();

    MPI_Barrier(MPI_COMM_WORLD);
}

void World2f::init_opencl() {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    if (platforms.empty()) {
        throw std::runtime_error("No OpenCL platforms found.");
    }

    cl::Platform platform = platforms.front();

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    if (devices.empty()) {
        throw std::runtime_error("No GPU devices found.");
    }

    cl::Device device = devices.front();

    context = cl::Context({device});
    queue = cl::CommandQueue(context, device);

    std::ifstream kernel_file("kernel/lenard_jhones.cl");
    std::string kernel_source((std::istreambuf_iterator<char>(kernel_file)),
                              std::istreambuf_iterator<char>());
    cl::Program::Sources sources;
    sources.push_back({kernel_source.c_str(), kernel_source.length()});
    program = cl::Program(context, sources);
    if (program.build({device}) != CL_SUCCESS) {
        throw std::runtime_error("Failed to build OpenCL program.");
    }

    kernel = cl::Kernel(program, "compute_forces");
}

void World2f::run_opencl_kernel() {
    cl::Buffer positions_x_buffer(context,
                                  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                  sizeof(float) * N, positions.x.data());
    cl::Buffer positions_y_buffer(context,
                                  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                  sizeof(float) * N, positions.y.data());
    cl::Buffer velocities_x_buffer(context,
                                   CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                   sizeof(float) * N, velocities.x.data());
    cl::Buffer velocities_y_buffer(context,
                                   CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                   sizeof(float) * N, velocities.y.data());

    kernel.setArg(0, positions_x_buffer);
    kernel.setArg(1, positions_y_buffer);
    kernel.setArg(2, velocities_x_buffer);
    kernel.setArg(3, velocities_y_buffer);
    kernel.setArg(4, 5.0f);   // sigma
    kernel.setArg(5, 1.0f);   // epsilon
    kernel.setArg(6, 400.0f); // threshold
    kernel.setArg(7, N);

    cl::NDRange global(N);
    cl::NDRange local(1);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);
    queue.finish();

    queue.enqueueReadBuffer(velocities_x_buffer, CL_TRUE, 0, sizeof(float) * N,
                            velocities.x.data());
    queue.enqueueReadBuffer(velocities_y_buffer, CL_TRUE, 0, sizeof(float) * N,
                            velocities.y.data());
}

void World2f::partition_particles() {
    float x_range = max_x - min_x;
    float x_per_process = x_range / mpi_size;
    float local_min_x = min_x + mpi_rank * x_per_process;
    float local_max_x = local_min_x + x_per_process;

    std::vector<float> new_positions_x;
    std::vector<float> new_positions_y;
    std::vector<float> new_velocities_x;
    std::vector<float> new_velocities_y;

    for (size_t i = 0; i < N; ++i) {
        if (positions.x[i] >= local_min_x && positions.x[i] < local_max_x) {
            new_positions_x.push_back(positions.x[i]);
            new_positions_y.push_back(positions.y[i]);
            new_velocities_x.push_back(velocities.x[i]);
            new_velocities_y.push_back(velocities.y[i]);
        }
    }

    positions.x = new_positions_x;
    positions.y = new_positions_y;
    velocities.x = new_velocities_x;
    velocities.y = new_velocities_y;

    local_N = positions.x.size();
}

void World2f::exchange_boundary_data() {
    MPI_Request request;
    MPI_Status status;

    if (mpi_rank < mpi_size - 1) {
        MPI_Isend(&positions.x[local_N - 1], 1, MPI_FLOAT, mpi_rank + 1, 0,
                  mpi_comm, &request);
        MPI_Wait(&request, &status);
    }

    if (mpi_rank > 0) {
        MPI_Irecv(&positions.x[0], 1, MPI_FLOAT, mpi_rank - 1, 0, mpi_comm,
                  &request);
        MPI_Wait(&request, &status);
    }
}
