#pragma once

#include <mutex>
#include <particle.hh>
#include <thread>
#include <vector>

struct Domain {
    std::vector<Particle *> particles;
    std::mutex mtx; // Mutex to guard access to the domain's particles
};

void update_domain(Domain &domain, double time_step,
                   Eigen::Vector3d (*interaction)(const Particle &,
                                                  const Particle &),
                   std::vector<Domain> &domains) {
    for (auto &p1 : domain.particles) {
        Eigen::Vector3d force = Eigen::Vector3d::Zero();

        // Interact with particles in the same domain
        for (auto &p2 : domain.particles) {
            if (p1 != p2) {
                force += interaction(*p1, *p2);
            }
        }

        // Interact with particles in neighboring domains
        for (auto &neighbor : domains) {
            if (&neighbor != &domain) {
                std::lock_guard<std::mutex> lock(neighbor.mtx);
                for (auto &p2 : neighbor.particles) {
                    force += interaction(*p1, *p2);
                }
            }
        }

        // Update particle state
        p1->velocity += (force / p1->mass) * time_step;
        p1->position += p1->velocity * time_step;
    }
}

void update_particles(std::vector<Particle> &particles, double time_step,
                      Eigen::Vector3d (*interaction)(const Particle &,
                                                     const Particle &),
                      int num_domains) {
    size_t n = particles.size();
    std::vector<Domain> domains(num_domains);

    for (size_t i = 0; i < n; ++i) {
        domains[i % num_domains].particles.push_back(&particles[i]);
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < num_domains; ++i) {
        threads.emplace_back(update_domain, std::ref(domains[i]), time_step,
                             interaction, std::ref(domains));
    }

    for (auto &t : threads) {
        t.join();
    }
}