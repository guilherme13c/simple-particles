CC := g++
CFLAGS := -Wextra -O3 -Iinclude
SOLVER_CFLAGS := $(CFLAGS) -Ieigen-3.4.0 -Iboost_1_86_0/build/include -Lboost_1_86_0/build/lib -l:libboost_program_options.a
GENERATE_VIDEO_CFLAGS := $(CFLAGS) -std=c++11 -lglfw -lGLEW -lGL -lGLU -Ieigen-3.4.0 -Iboost_1_86_0/build/include -Lboost_1_86_0/build/lib -l:libboost_program_options.a

BUILD_DIR := bin
INCLUDE_DIR := include
DUMP_DIR := dump

GENERATE_VIDEO_SRC := generate_video.cc
GENERATE_VIDEO_TARGET := $(BUILD_DIR)/generate-video

SOLVER_SRC := main.cc
SOLVER_TARGET := $(BUILD_DIR)/solver

INPUT_FILE = "init_state.txt"
DUMP_FILE = "dump/simulation.dump"

.PHONY: all build build-generate-video clean run

all: $(SOLVER_TARGET) $(GENERATE_VIDEO_TARGET)

$(SOLVER_TARGET): $(SOLVER_SRC)
	$(CC) $(SOLVER_SRC) -o $(SOLVER_TARGET) $(SOLVER_CFLAGS)

$(GENERATE_VIDEO_TARGET): $(GENERATE_VIDEO_SRC)
	$(CC) $(GENERATE_VIDEO_SRC) -o $(GENERATE_VIDEO_TARGET) $(GENERATE_VIDEO_CFLAGS)

clean:
	rm -rf $(BUILD_DIR)/* $(DUMP_DIR)/*

run: $(SOLVER_TARGET) $(GENERATE_VIDEO_TARGET)
	$(SOLVER_TARGET) -i $(INPUT_FILE) -o $(DUMP_FILE)
	$(GENERATE_VIDEO_TARGET) -i $(DUMP_FILE)
