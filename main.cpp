#include <2f.h>

int main(int argc, char **argv) {
    assert(argc == 2);

    World2f simulation(std::atoi(argv[1]), 100, -100, 100, -100, 10, 0.01, 10);

    simulation.set_damping_factor(0.01);

    simulation.run();

    return 0;
}
