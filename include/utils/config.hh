#pragma once

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/type.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>

struct SimulationConfig {
    uint64_t particle_count;     // -N
    uint16_t world_width;        // -W
    uint16_t world_height;       // -H
    std::ifstream initial_state; // -i
    std::ofstream dump_file;     // -o
    uint64_t num_steps;          // -s
    float time_step;             // -dt
};

void parse_simulation_config(struct SimulationConfig &cfg, int argc,
                             char *argv[]) {
    namespace po = boost::program_options;

    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")(
        "particle-count,N",
        po::value<uint64_t>(&cfg.particle_count)->required(),
        "number of particles")(
        "width,W", po::value<uint16_t>(&cfg.world_width)->required(),
        "world width")("height,H",
                       po::value<uint16_t>(&cfg.world_height)->required(),
                       "world height")("input,i", po::value<std::string>(),
                                       "input file with initial state")(
        "dump,o", po::value<std::string>(), "file to dump simulation results")(
        "step-count,s", po::value<uint64_t>(&cfg.num_steps)->required(),
        "number of simulation steps")(
        "time-step,t", po::value<float>(&cfg.time_step)->required(),
        "time step size");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return;
        }

        po::notify(vm);

        if (vm.count("input")) {
            cfg.initial_state.open(vm["input"].as<std::string>(), std::ios::in);
            if (!cfg.initial_state.is_open()) {
                throw std::runtime_error("Failed to open input file");
            }
        }

        if (vm.count("dump")) {
            cfg.dump_file.open(vm["dump"].as<std::string>(), std::ios::out);
            if (!cfg.dump_file.is_open()) {
                throw std::runtime_error("Failed to open dump file");
            }
        }
    } catch (const po::error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << desc << std::endl;
        throw;
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        throw;
    }
}

void read_initial_state(std::ifstream &input,
                        std::vector<Particle> &particles) {
    double x, y, z, vx, vy, vz, mass;
    while (input >> x >> y >> z >> vx >> vy >> vz >> mass) {
        Particle p;
        p.position = Eigen::Vector3d(x, y, z);
        p.velocity = Eigen::Vector3d(vx, vy, vz);
        p.mass = mass;
        particles.push_back(p);
    }
}
