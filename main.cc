#include <Eigen/Dense>
#include <fstream>
#include <iostream>
#include <particle.hh>
#include <utils/config.hh>
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

int main(int argc, char **argv) {
    struct SimulationConfig cfg;
    parse_simulation_config(cfg, argc, argv);

    std::vector<Particle> particles;
    if (cfg.initial_state.is_open()) {
        read_initial_state(cfg.initial_state, particles);
    } else {
        std::cerr << "Error: Initial state file is not open." << std::endl;
        return 1;
    }

    for (u_int64_t step = 0; step < cfg.num_steps; ++step) {
        update_particles(particles, cfg.time_step,
                         calculate_gravitational_force);
        save_simulation_state(particles, cfg.dump_file);
    }

    cfg.dump_file.close();
    return 0;
}
