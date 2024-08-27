#include <2f.h>
#include <iostream>

void kernel(Particle2f &p1, Particle2f &p2) {
    Eigen::Vector2f diff = p2.get_position() - p1.get_position();
    float dist = diff.norm();

    if (dist > 100)
        return;

    if (dist < 1e-6f) {
        dist = 1e-6f;
    }

    float force_magnitude = 1000.0f / (dist * dist);

    Eigen::Vector2f force_vector = force_magnitude * diff.normalized();

    p1.get_velocity() -= force_vector;
    p2.get_velocity() += force_vector;
}

int main(int argc, char **argv) {
    World2f simulation(400, 100, -100, 100, -100, 0, 0.1, 10, true,
                       "world.dump");

    simulation.set_interaction_kernel(kernel);
    simulation.set_damping_factor(0.15);

    simulation.run();

    return 0;
}
