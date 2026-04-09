#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

echo "[1/2] Building engine_demo..."
clang++ -std=c++20 -Iinclude \
  src/main.cpp \
  src/engine/AudioBuffer.cpp src/engine/AudioEngine.cpp src/engine/AudioMetrics.cpp src/engine/TestToneGenerator.cpp src/engine/WavWriter.cpp \
  src/synth/SynthVoice.cpp src/effects/EffectsChain.cpp src/sequencer/Sequencer.cpp src/persistence/ProjectSerializer.cpp \
  -o engine_demo

echo "[2/2] Running engine_demo..."
./engine_demo

echo ""
echo "Done. Generated files:"
echo " - demo_render.wav"
echo " - demo_project.json"
echo ""
echo "To listen on macOS: afplay demo_render.wav"
