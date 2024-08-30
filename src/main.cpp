#include <2f.h>

int main(const int argc, char **argv) {
    assert(argc == 2);

    const uint64_t N = std::strtol(argv[1], nullptr, 10);

    World2f simulation(N, 1000, -1000, 1000, -1000, 3, 0.01, 10, 10, 10);

    simulation.set_damping_factor(0.15);

    simulation.run();

    return 0;
}
