# Test notes

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
# or: make
```

## Default run (all assessment checks)

```bash
./build/engine_demo
```

Prints three sections to the terminal:

1. **Buffer sizes** — render at 128, 256, 512, and full offline frame count (960000)
2. **MIDI @ 50 BPM** — short summary + a few sample events (`grid=ok` / `pattern=ok`). Full event list: `--verify-midi`
3. **20 s WAV** — writes `demo_render.wav`, validates 48 kHz stereo PCM-16 (~20 s)

Ends with **ALL CHECKS PASSED** or **SOME CHECKS FAILED** (exit 0 / 1).

## Optional flags

| Command | What it runs |
|---------|----------------|
| `./build/engine_demo --test-buffers` | Check 1 only |
| `./build/engine_demo --verify-midi` | Check 2 only |
| `./build/engine_demo --demo-only` | Check 3 only |

## Manual step

Open `demo_render.wav` in QuickTime / Audacity to confirm it **sounds** correct (terminal only validates file format and duration).

## Results (fill in locally)

- Date / machine:
- `./build/engine_demo`: pass / fail
- Playback spot-check: ok / issue
