#include <iostream>
// #include <mpi.h>
#include <particle.h>
#include <world.h>

int main(int argc, char **argv) {
    auto particles = std::vector<Particle3f>(
        {Particle3f(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 0, 0))});

    World simulation(100, -100, 100, -100, 100, -100, 10, particles, 0.01, true,
                     "world.dump");

    for (auto i = 0; i < 3; i++) {
        simulation.update_particles();
    }

    return 0;
}
