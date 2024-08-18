#pragma once

#include <fstream>
#include <particle.hh>
#include <utils/config.hh>
#include <vector>

void save_simulation_state(const std::vector<Particle> &particles,
                           std::ofstream &file) {
    for (const auto &particle : particles) {
        file.write(reinterpret_cast<const char *>(particle.position.data()),
                   sizeof(double) * 3);
        file.write(reinterpret_cast<const char *>(particle.velocity.data()),
                   sizeof(double) * 3);
        file.write(reinterpret_cast<const char *>(&particle.mass),
                   sizeof(double));
    }
}

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

void save_simulation_config(std::ofstream &file,
                            const struct SimulationConfig &cfg) {
    file.write(reinterpret_cast<const char *>("SIMPLE"), sizeof(char) * 6);
    file.write(reinterpret_cast<const char *>(&cfg.num_steps),
               sizeof(uint64_t));
    file.write(reinterpret_cast<const char *>(&cfg.particle_count),
               sizeof(uint64_t));
    file.write(reinterpret_cast<const char *>(&cfg.time_step), sizeof(float));
    file.write(reinterpret_cast<const char *>(&cfg.world_width),
               sizeof(uint16_t));
    file.write(reinterpret_cast<const char *>(&cfg.world_height),
               sizeof(uint16_t));
}

bool read_simulation_config(std::ifstream &file, struct SimulationConfig &cfg) {
    char header_signature[6];
    file.read(reinterpret_cast<char *>(header_signature), sizeof(char) * 6);

    if (strncmp("SIMPLE", header_signature, 6) != 0) {
        return false;
    }

    file.read(reinterpret_cast<char *>(&cfg.num_steps), sizeof(uint64_t));
    file.read(reinterpret_cast<char *>(&cfg.particle_count), sizeof(uint64_t));
    file.read(reinterpret_cast<char *>(&cfg.time_step), sizeof(float));
    file.read(reinterpret_cast<char *>(&cfg.world_width), sizeof(uint16_t));
    file.read(reinterpret_cast<char *>(&cfg.world_height), sizeof(uint16_t));

    return true;
}
