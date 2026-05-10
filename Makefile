# Build without CMake (e.g. macOS with only Xcode CLT). For JUCE support, install CMake and use CMakeLists.txt.
BUILD_DIR := build
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -O2 -I include

ENGINE_SRCS := \
	src/engine/AudioBuffer.cpp \
	src/engine/AudioEngine.cpp \
	src/engine/AudioMetrics.cpp \
	src/engine/TestToneGenerator.cpp \
	src/engine/WavWriter.cpp \
	src/synth/SynthVoice.cpp \
	src/effects/EffectsChain.cpp \
	src/sequencer/Sequencer.cpp \
	src/persistence/ProjectSerializer.cpp

OBJS := $(ENGINE_SRCS:src/%.cpp=$(BUILD_DIR)/%.o) $(BUILD_DIR)/main.o

.PHONY: all clean

all: $(BUILD_DIR)/engine_demo

$(BUILD_DIR)/engine_demo: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
