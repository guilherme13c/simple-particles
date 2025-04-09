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
constexpr unsigned long size = 10000;

int main() {
    auto oclw = OpenClWrapper();

    cl::Program program;

    program = cl::Program(
        oclw.context, oclw.load_program("kernel/update_positions.cl"), true);

    cl::Kernel update_positions(program, "update_positions");

    cl::Buffer p_buffer(oclw.context, CL_MEM_READ_WRITE,
                        sizeof(SDL_FPoint) * size);
    cl::Buffer v_buffer(oclw.context, CL_MEM_READ_WRITE,
                        sizeof(SDL_FPoint) * size);

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
    std::uniform_real_distribution<> distW(0, WIDTH), distH(0, HEIGHT),
        distV(-10, 10);

    std::vector<SDL_FPoint> p(size), v(size);

    for (auto i = 0; i < size; i++) {
        p[i].x = distW(rng);
        p[i].y = distH(rng);
        v[i].x = distV(rng);
        v[i].y = distV(rng);
    }

    cl::NDRange global(1024);
    cl::NDRange local(64);

    SDL_Event e;
    bool quit = false;

    auto lastTime = std::chrono::high_resolution_clock::now();
    std::chrono::_V2::system_clock::time_point currentTime;
    std::chrono::duration<float> elapsed;
    float deltaTime;

    while (!quit) {
        currentTime = std::chrono::high_resolution_clock::now();
        elapsed = currentTime - lastTime;
        lastTime = currentTime;
        deltaTime = elapsed.count();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }

        {
            cl::copy(oclw.queue, p.begin(), p.end(), p_buffer);
            cl::copy(oclw.queue, v.begin(), v.end(), v_buffer);

            update_positions.setArg(0, p_buffer);
            update_positions.setArg(1, v_buffer);
            update_positions.setArg(2, deltaTime);

            oclw.queue.enqueueNDRangeKernel(update_positions, cl::NullRange,
                                            global, local);

            oclw.queue.finish();

            cl::copy(oclw.queue, p_buffer, p.begin(), p.end());
        }

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);
        {
            SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
            SDL_RenderPoints(ren, p.data(), p.size());
        }
        SDL_RenderPresent(ren);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return EXIT_SUCCESS;
}
