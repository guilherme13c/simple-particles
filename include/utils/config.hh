#pragma once

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/type.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>

struct SimulationConfig {
    uint64_t particle_count;
    uint64_t num_steps;
    float time_step;
    uint16_t world_width;
    uint16_t world_height;
    std::ifstream initial_state; // -i
    std::ofstream dump_file;     // -o
};

void parse_simulation_config(struct SimulationConfig &cfg, int argc,
                             char *argv[]) {
    namespace po = boost::program_options;

    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")(
        "input,i", po::value<std::string>()->required(),
        "input file with initial state")("dump,o",
                                         po::value<std::string>()->required(),
                                         "file to dump simulation results");

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

void read_initial_state(std::ifstream &input, std::vector<Particle> &particles,
                        struct SimulationConfig &cfg) {
    input >> cfg.particle_count >> cfg.num_steps >> cfg.time_step >>
        cfg.world_width >> cfg.world_height;

    double x, y, z, vx, vy, vz, mass;
    while (input >> x >> y >> z >> vx >> vy >> vz >> mass) {
        Particle p;
        p.position = Eigen::Vector3d(x, y, z);
        p.velocity = Eigen::Vector3d(vx, vy, vz);
        p.mass = mass;
        particles.push_back(p);
    }
}

struct VideoGenerationConfig {
    std::ifstream dump_file; // -i
};

void parse_video_generation_config(struct VideoGenerationConfig &cfg, int argc,
                                   char *argv[]) {
    namespace po = boost::program_options;

    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "produce help message")(
        "input,i", po::value<std::string>()->required(),
        "input file with dump of the states");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return;
        }

        po::notify(vm);

        if (vm.count("input")) {
            cfg.dump_file.open(vm["input"].as<std::string>(), std::ios::in);
            if (!cfg.dump_file.is_open()) {
                throw std::runtime_error("Failed to open input file");
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
