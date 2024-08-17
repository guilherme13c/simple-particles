#pragma once

#include <fstream>
#include <particle.hh>
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
