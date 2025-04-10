#include "opencl_utils.hpp"

#include <SDL3/SDL.h>

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <random>

constexpr int WIDTH = 1200;
constexpr int HEIGHT = 900;
constexpr unsigned long size = 100;
constexpr SDL_FPoint limits = {.x = WIDTH, .y = HEIGHT};

int main() {
    auto oclw = OpenClWrapper();

    cl::Program program;

    program = cl::Program(
        oclw.context, oclw.load_program("kernel/update_positions.cl"), true);

    cl::Kernel update_positions_kernel(program, "update_positions");

    cl::Buffer positions_buffer(oclw.context, CL_MEM_READ_WRITE,
                                sizeof(SDL_FPoint) * size);
    cl::Buffer velocities_buffer(oclw.context, CL_MEM_READ_WRITE,
                                 sizeof(SDL_FPoint) * size);

    program = cl::Program(
        oclw.context, oclw.load_program("kernel/pair_differences.cl"), true);

    cl::Kernel pair_differences_kernel(program, "pair_differences");

    cl::Buffer distances_buffer(oclw.context, CL_MEM_READ_WRITE,
                                sizeof(SDL_FPoint) * size * size);

    program = cl::Program(oclw.context,
                          oclw.load_program("kernel/compute_forces.cl"), true);

    cl::Kernel compute_forces(program, "compute_forces");

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *win = SDL_CreateWindow("Simple Particles", WIDTH, HEIGHT, 0);
    if (win == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, NULL);
    if (ren == nullptr) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError()
                  << std::endl;
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<> distW(0, WIDTH), distH(0, HEIGHT);

    std::vector<SDL_FPoint> positions(size), velocities(size);

    for (auto i = 0; i < size; i++) {
        positions[i].x = distW(rng);
        positions[i].y = distH(rng);
        velocities[i].x = 0;
        velocities[i].y = 0;
    }

    cl::NDRange global(4096);
    cl::NDRange local(128);

    SDL_Event e;
    bool quit = false;

    auto lastTime = std::chrono::high_resolution_clock::now();
    std::chrono::_V2::system_clock::time_point currentTime;
    std::chrono::duration<float> elapsed;
    float deltaTime;

    while (!quit) {
        {
            currentTime = std::chrono::high_resolution_clock::now();
            elapsed = currentTime - lastTime;
            lastTime = currentTime;
            deltaTime = elapsed.count();
        }

        {
            cl::copy(oclw.queue, positions.begin(), positions.end(),
                     positions_buffer);
            cl::copy(oclw.queue, velocities.begin(), velocities.end(),
                     velocities_buffer);

            pair_differences_kernel.setArg(0, size);
            pair_differences_kernel.setArg(1, positions_buffer);
            pair_differences_kernel.setArg(2, distances_buffer);

            oclw.queue.enqueueNDRangeKernel(pair_differences_kernel,
                                            cl::NullRange, global, local);

            compute_forces.setArg(0, size);
            compute_forces.setArg(1, distances_buffer);
            compute_forces.setArg(2, velocities_buffer);
            compute_forces.setArg(3, deltaTime);

            oclw.queue.enqueueNDRangeKernel(compute_forces, cl::NullRange,
                                            global, local);

            update_positions_kernel.setArg(0, positions_buffer);
            update_positions_kernel.setArg(1, velocities_buffer);
            update_positions_kernel.setArg(2, deltaTime);
            update_positions_kernel.setArg(3, limits);

            oclw.queue.enqueueNDRangeKernel(update_positions_kernel,
                                            cl::NullRange, global, local);

            cl::copy(oclw.queue, positions_buffer, positions.begin(),
                     positions.end());

            oclw.queue.finish();
        }

        {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                }
            }
        }

        {
            SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
            SDL_RenderClear(ren);

            SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
            SDL_RenderPoints(ren, positions.data(), positions.size());

            SDL_RenderPresent(ren);
        }
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return EXIT_SUCCESS;
}
