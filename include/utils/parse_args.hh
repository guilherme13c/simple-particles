#pragma once

#include <boost/program_options.hpp>
#include <boost/type.hpp>
#include <string>

struct Config {
    uint64_t particle_count;
    uint16_t world_width;
    uint16_t world_height;
    std::string initial_state;
};
