cmake_minimum_required(VERSION 3.22)
project(FlStyleAudioEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AUDIO_ENGINE_ENABLE_JUCE "Enable JUCE audio device integration" OFF)

add_library(audio_engine
    src/engine/AudioBuffer.cpp
    src/engine/AudioEngine.cpp
    src/engine/AudioMetrics.cpp
    src/engine/TestToneGenerator.cpp
    src/engine/WavWriter.cpp
    src/synth/SynthVoice.cpp
    src/effects/EffectsChain.cpp
    src/sequencer/Sequencer.cpp
    src/persistence/ProjectSerializer.cpp
)

target_include_directories(audio_engine PUBLIC include)
target_compile_options(audio_engine PRIVATE -Wall -Wextra -Wpedantic)

if(AUDIO_ENGINE_ENABLE_JUCE)
    include(FetchContent)
    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.2
    )
    FetchContent_MakeAvailable(juce)

    target_compile_definitions(audio_engine PUBLIC AUDIO_ENGINE_HAS_JUCE=1)
    target_link_libraries(audio_engine PUBLIC juce::juce_audio_devices juce::juce_audio_basics)
endif()

add_executable(engine_demo src/main.cpp)
target_link_libraries(engine_demo PRIVATE audio_engine)
