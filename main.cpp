#include <2f.h>

int main(const int argc, char **argv) {
    assert(argc == 2);

    const uint64_t N = std::strtol(argv[1], nullptr, 10);

    World2f simulation(N, 100, -100, 100, -100, 10, 0.01, 10);

    simulation.set_damping_factor(0.01);

    simulation.run();

    return 0;
}
