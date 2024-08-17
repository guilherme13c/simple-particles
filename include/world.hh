#pragma once

#include <particle.hh>
#include <vector>

void update_particles(std::vector<Particle> &particles, double time_step,
                      Eigen::Vector3d (*interaction)(const Particle &,
                                                     const Particle &)) {
    size_t n = particles.size();
    std::vector<Eigen::Vector3d> forces(n, Eigen::Vector3d::Zero());

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            Eigen::Vector3d force = interaction(particles[i], particles[j]);
            forces[i] += force;
            forces[j] -= force;
        }
    }

    for (size_t i = 0; i < n; ++i) {
        Eigen::Vector3d acceleration = forces[i] / particles[i].mass;
        particles[i].velocity += acceleration * time_step;
        particles[i].position += particles[i].velocity * time_step;
    }
}
