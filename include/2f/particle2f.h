#pragma once

#include <Eigen/Dense>
#include <random>

class Particle2f {
  private:
    Eigen::Vector2f position;
    Eigen::Vector2f velocity;

    static std::mt19937 rng;
    static std::uniform_real_distribution<float> dist_pos_x;
    static std::uniform_real_distribution<float> dist_pos_y;
    static std::uniform_real_distribution<float> dist_vel_x;
    static std::uniform_real_distribution<float> dist_vel_y;

  public:
    Particle2f(float min_x, float max_x, float min_y, float max_y,
               float min_velocity, float max_velocity) {
        position.x() = dist_pos_x(rng) * (max_x - min_x) + min_x;
        position.y() = dist_pos_y(rng) * (max_y - min_y) + min_y;

        velocity.x() =
            dist_vel_x(rng) * (max_velocity - min_velocity) + min_velocity;
        velocity.y() =
            dist_vel_y(rng) * (max_velocity - min_velocity) + min_velocity;
    }

    Particle2f(const Eigen::Vector2f &pos, const Eigen::Vector2f &vel)
        : position(pos), velocity(vel) {}

    Particle2f() : position(Eigen::Vector2f()), velocity(Eigen::Vector2f()) {}

    inline Eigen::Vector2f &get_position(void) { return this->position; }
    inline Eigen::Vector2f &get_velocity(void) { return this->velocity; }

    inline float get_speed(void) { return this->velocity.norm(); }
};

// Static member definitions
std::mt19937
    Particle2f::rng(std::random_device{}()); // Seed with a random device
std::uniform_real_distribution<float> Particle2f::dist_pos_x(0.0f, 1.0f);
std::uniform_real_distribution<float> Particle2f::dist_pos_y(0.0f, 1.0f);
std::uniform_real_distribution<float> Particle2f::dist_vel_x(0.0f, 1.0f);
std::uniform_real_distribution<float> Particle2f::dist_vel_y(0.0f, 1.0f);
