#pragma once

#include <Eigen/Dense>
#include <random>

class Particle3f {
  private:
    Eigen::Vector3f position;
    Eigen::Vector3f velocity;

    static std::mt19937 rng;
    static std::uniform_real_distribution<float> dist_pos_x;
    static std::uniform_real_distribution<float> dist_pos_y;
    static std::uniform_real_distribution<float> dist_pos_z;
    static std::uniform_real_distribution<float> dist_vel_x;
    static std::uniform_real_distribution<float> dist_vel_y;
    static std::uniform_real_distribution<float> dist_vel_z;

  public:
    Particle3f(float min_x, float max_x, float min_y, float max_y, float min_z,
               float max_z, float min_velocity, float max_velocity) {
        position.x() = dist_pos_x(rng) * (max_x - min_x) + min_x;
        position.y() = dist_pos_y(rng) * (max_y - min_y) + min_y;
        position.z() = dist_pos_z(rng) * (max_z - min_z) + min_z;

        velocity.x() =
            dist_vel_x(rng) * (max_velocity - min_velocity) + min_velocity;
        velocity.y() =
            dist_vel_y(rng) * (max_velocity - min_velocity) + min_velocity;
        velocity.z() =
            dist_vel_z(rng) * (max_velocity - min_velocity) + min_velocity;
    }

    Particle3f(const Eigen::Vector3f &pos, const Eigen::Vector3f &vel)
        : position(pos), velocity(vel) {}

    Particle3f() : position(Eigen::Vector3f()), velocity(Eigen::Vector3f()) {}

    inline Eigen::Vector3f &get_position(void) { return this->position; }
    inline Eigen::Vector3f &get_velocity(void) { return this->velocity; }

    inline float get_speed(void) { return this->velocity.norm(); }
};

// Static member definitions
std::mt19937
    Particle3f::rng(std::random_device{}()); // Seed with a random device
std::uniform_real_distribution<float> Particle3f::dist_pos_x(0.0f, 1.0f);
std::uniform_real_distribution<float> Particle3f::dist_pos_y(0.0f, 1.0f);
std::uniform_real_distribution<float> Particle3f::dist_pos_z(0.0f, 1.0f);
std::uniform_real_distribution<float> Particle3f::dist_vel_x(0.0f, 1.0f);
std::uniform_real_distribution<float> Particle3f::dist_vel_y(0.0f, 1.0f);
std::uniform_real_distribution<float> Particle3f::dist_vel_z(0.0f, 1.0f);
