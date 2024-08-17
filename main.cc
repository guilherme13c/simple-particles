#include <Eigen/Dense>
#include <fstream>
#include <iostream>
#include <particle.hh>
#include <utils/save.hh>
#include <vector>
#include <world.hh>

// Constants
const double G = 6.67430e-11; // Gravitational constant

Eigen::Vector3d calculate_gravitational_force(const Particle &p1,
                                              const Particle &p2) {
    Eigen::Vector3d direction = p2.position - p1.position;
    double distance = direction.norm();
    if (distance == 0)
        return Eigen::Vector3d::Zero(); // Avoid division by zero
    direction.normalize();
    double force_magnitude = G * p1.mass * p2.mass / (distance * distance);
    return force_magnitude * direction;
}

int main() {
    std::vector<Particle> particles = {
        Particle(Eigen::Vector3d(0, 0, 0), Eigen::Vector3d(0, 0, 0), 1e10),
        Particle(Eigen::Vector3d(1, 0, 0), Eigen::Vector3d(0, 1, 0), 1e10)};

    double time_step = 0.01;
    int num_steps = 1000;

    std::ofstream dump_file("dump/simulation.bin", std::ios::binary);

    for (int step = 0; step < num_steps; ++step) {
        update_particles(particles, time_step, calculate_gravitational_force);
        save_simulation_state(particles, dump_file);
    }

    dump_file.close();
    return 0;
}
