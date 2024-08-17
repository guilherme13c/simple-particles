#include <Eigen/Dense>
#include <fstream>
#include <iostream>
#include <vector>

// Constants
const double G = 6.67430e-11; // Gravitational constant

// Particle structure
struct Particle {
    Eigen::Vector3d position;
    Eigen::Vector3d velocity;
    double mass;

    Particle(const Eigen::Vector3d &pos, const Eigen::Vector3d &vel, double m)
        : position(pos), velocity(vel), mass(m) {}
};

// Function to calculate gravitational force between two particles
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

// Function to update the particles
void update_particles(std::vector<Particle> &particles, double time_step,
                      Eigen::Vector3d (*interaction)(const Particle &,
                                                     const Particle &)) {
    size_t n = particles.size();
    std::vector<Eigen::Vector3d> forces(n, Eigen::Vector3d::Zero());

    // Calculate forces on each particle
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            Eigen::Vector3d force = interaction(particles[i], particles[j]);
            forces[i] += force;
            forces[j] -= force; // Newton's third law
        }
    }

    // Update positions and velocities
    for (size_t i = 0; i < n; ++i) {
        Eigen::Vector3d acceleration = forces[i] / particles[i].mass;
        particles[i].velocity += acceleration * time_step;
        particles[i].position += particles[i].velocity * time_step;
    }
}

// Function to save the simulation state in a binary file
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

int main() {
    // Example particles
    std::vector<Particle> particles = {
        Particle(Eigen::Vector3d(0, 0, 0), Eigen::Vector3d(0, 0, 0), 1e10),
        Particle(Eigen::Vector3d(1, 0, 0), Eigen::Vector3d(0, 1, 0), 1e10)};

    // Simulation parameters
    double time_step = 0.01;
    int num_steps = 1000;

    // Open a binary file to dump the simulation data
    std::ofstream dump_file("dump/simulation.bin", std::ios::binary);

    // Run the simulation
    for (int step = 0; step < num_steps; ++step) {
        update_particles(particles, time_step, calculate_gravitational_force);
        save_simulation_state(particles, dump_file);
    }

    dump_file.close();
    return 0;
}
