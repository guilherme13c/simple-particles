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
