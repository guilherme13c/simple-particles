#include <2f.h>
#include <iostream>

int main(int argc, char **argv) {
    World2f simulation(10, 100, -100, 100, -100, 1000, 0.001, true,
                       "world.dump");

    for (auto i = 0; i < 1000; i++) {
        simulation.update_particles();
    }

    return 0;
}
