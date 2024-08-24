#pragma once

#include <atomic>
#include <cinttypes>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <2f/particle2f.h>

class World2f {
  private:
    float max_x;
    float min_x;
    float max_y;
    float min_y;
    float max_vel;

    std::vector<Particle2f> particles;

    float dt;
    bool save_states;

    std::string dump_file_name;
    std::ofstream dump_file;
    std::queue<std::vector<char>> state_queue;
    std::mutex queue_mutex;
    std::condition_variable cv;
    std::thread worker_thread;
    std::atomic<bool> stop_worker{false};

    void create_random_particles(size_t N) {
        particles.clear();    // Clear existing particles
        particles.reserve(N); // Reserve space for efficiency

        for (size_t i = 0; i < N; ++i) {
            particles.emplace_back(min_x, max_x, min_y, max_y, -max_vel,
                                   max_vel);
        }
    }

    void save_state_to_file(void) {
        while (!stop_worker || !state_queue.empty()) {
            std::vector<char> state_data;

            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                cv.wait(lock,
                        [this] { return stop_worker || !state_queue.empty(); });

                if (state_queue.empty() && stop_worker) {
                    break;
                }

                if (!state_queue.empty()) {
                    state_data = std::move(state_queue.front());
                    state_queue.pop();
                }
            }

            if (!state_data.empty()) {
                dump_file.write(state_data.data(), state_data.size());
            }
        }
    }

    void save_configuration(void) {
        if (!save_states)
            return;

        // Open the file in binary mode for writing the configuration
        dump_file.open(dump_file_name, std::ios::binary);
        if (!dump_file.is_open()) {
            throw std::runtime_error("Unable to open configuration file.");
        }

        uint64_t N = particles.size();

        // Write configuration to the file
        dump_file.write(reinterpret_cast<const char *>(&N), sizeof(N));
        dump_file.write(reinterpret_cast<const char *>(&dt), sizeof(dt));
        dump_file.write(reinterpret_cast<const char *>(&max_x), sizeof(max_x));
        dump_file.write(reinterpret_cast<const char *>(&min_x), sizeof(min_x));
        dump_file.write(reinterpret_cast<const char *>(&max_y), sizeof(max_y));
        dump_file.write(reinterpret_cast<const char *>(&min_y), sizeof(min_y));

        dump_file.flush();
    }

  public:
    World2f(float max_x, float min_x, float max_y, float min_y, float max_vel,
            std::vector<Particle2f> &particles, float dt,
            bool save_states = false, const std::string dump_file_name = "")
        : max_x(max_x), min_x(min_x), max_y(max_y), min_y(min_y),
          max_vel(max_vel), particles(particles), dt(dt),
          save_states(save_states), dump_file_name(dump_file_name) {
        if (save_states) {
            assert(dump_file_name != "");
            save_configuration();
            stop_worker = false;
            worker_thread = std::thread(&World2f::save_state_to_file, this);
        }
    }

    World2f(uint64_t N, float max_x, float min_x, float max_y, float min_y,
            float max_vel, float dt, bool save_states = false,
            const std::string dump_file_name = "")
        : max_x(max_x), min_x(min_x), max_y(max_y), min_y(min_y),
          max_vel(max_vel), dt(dt), save_states(save_states),
          dump_file_name(dump_file_name) {
        create_random_particles(N);
        if (save_states) {
            assert(dump_file_name != "");
            save_configuration();
            stop_worker = false;
            worker_thread = std::thread(&World2f::save_state_to_file, this);
        }
    }

    inline float &get_max_x(void) { return this->max_x; }
    inline float &get_min_x(void) { return this->min_x; }

    inline float &get_max_y(void) { return this->max_y; }
    inline float &get_min_y(void) { return this->min_y; }

    void update_particles(void) {
        for (auto &p : this->particles) {
            p.get_position() += p.get_velocity() * this->dt;

            auto &pos = p.get_position();
            auto &vel = p.get_velocity();
            if (pos.x() < min_x) {
                pos.x() = min_x;
                vel.x() *= -1;
            }
            if (pos.x() > max_x) {
                pos.x() = max_x;
                vel.x() *= -1;
            }
            if (pos.y() < min_y) {
                pos.y() = min_y;
                vel.y() *= -1;
            }
            if (pos.y() > max_y) {
                pos.y() = max_y;
                vel.y() *= -1;
            }
        }

        if (save_states) {
            std::vector<char> state_data;
            state_data.reserve(particles.size() * sizeof(Particle2f));

            for (const auto &p : particles) {
                state_data.insert(
                    state_data.end(), reinterpret_cast<const char *>(&p),
                    reinterpret_cast<const char *>(&p) + sizeof(Particle2f));
            }

            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                state_queue.push(std::move(state_data));
                cv.notify_one();
            }
        }
    }

    ~World2f() {
        if (save_states) {
            stop_worker = true;
            cv.notify_one();
            if (worker_thread.joinable()) {
                worker_thread.join();
            }
            if (dump_file.is_open()) {
                save_state_to_file();
                dump_file.close();
            }
        }
    }
};
