#pragma once

#define __CL_ENABLE_EXCEPTIONS

#include <fstream>
#include <iostream>

#include <CL/opencl.hpp>

class OpenClWrapper {
  public:
    cl::Context context;
    cl::CommandQueue queue;

    OpenClWrapper() {
        this->context = cl::Context(CL_DEVICE_TYPE_GPU);
        this->queue = cl::CommandQueue(context);
    }

    std::string load_program(std::string input) {
        std::ifstream stream(input.c_str());
        if (!stream.is_open()) {
            std::cout << "Cannot open file: " << input << std::endl;
            exit(1);
        }
        return std::string(std::istreambuf_iterator<char>(stream),
                           (std::istreambuf_iterator<char>()));
    }
};
