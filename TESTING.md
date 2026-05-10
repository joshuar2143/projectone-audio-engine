# Test notes

## Manual smoke test

| Step | Command | Expected result |
|------|---------|-----------------|
| Configure | `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release` | CMake finishes without errors |
| Build | `cmake --build build` | `engine_demo` links successfully |
| Run | `./build/engine_demo` | Exit code `0` |

## What the demo checks

- **Offline render:** Writes `demo_render.wav` (48 kHz, ~20 s of audio from one full `AudioEngine::process` pass: sequencer → synth → effects chain).
- **Project save:** Writes `demo_project.json` with a small note list.
- **Metrics:** Console prints `Render complete. Last callback ms: …` (time for that offline processing block).

## Results (fill in locally)

- Build: pass / fail — _date / machine_
- Run: pass / fail — _any errors or warnings_
- Output files: `demo_render.wav` and `demo_project.json` present — yes / no
- Audio spot-check (optional): open the WAV — _hear tone/pattern, clipping, silence, etc._

## Notes

- There is no separate automated unit-test target in CMake yet; this smoke test is the main repeatable check.
- For live device I/O, the project can be built with `-DAUDIO_ENGINE_ENABLE_JUCE=ON` (requires network for the JUCE fetch on first configure).
