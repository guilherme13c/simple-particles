#pragma once

#include <Eigen/Dense>

struct Particle {
    Eigen::Vector3d position;
    Eigen::Vector3d velocity;
    double mass;

    Particle(const Eigen::Vector3d &pos, const Eigen::Vector3d &vel, double m)
        : position(pos), velocity(vel), mass(m) {}

    Particle(void)
        : position(Eigen::Vector3d(0, 0, 0)),
          velocity(Eigen::Vector3d(0, 0, 0)), mass(0) {}
};
