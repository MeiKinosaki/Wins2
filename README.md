# Wins2

**PlayStation 2 emulator for Windows x64**, designed with a low-memory target (4 GB RAM) and no mandatory AVX/AVX2 requirement.

> Status: early development. This repository is a foundation, not yet a playable emulator. No game compatibility or performance claims are made until tested.

## Goals
- Windows 10/11 x64, portable distribution (no installer).
- Target systems with 4 GB RAM; actual requirements depend on the game and graphics backend.
- Baseline x86-64/SSE2 only; AVX/AVX2 optimizations must remain optional.
- PlayStation 2 disc images: ISO first, then BIN/CUE and CHD.
- Multi-disc library entries with explicit disc switching without restarting the emulation session.
- Game-specific graphics, speed, and memory settings.
- Save states and memory cards.

## Planned architecture
- Emotion Engine (MIPS R5900)
- I/O Processor (MIPS R3000A)
- VU0/VU1 vector units
- Graphics Synthesizer (GS)
- SPU2 audio
- DMA, timers, interrupts, CD/DVD controller
- BIOS interface and memory cards
- Portable desktop frontend and game library

## Build the current foundation
Requires CMake 3.20+ and a C++20 compiler.

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The current executable only reports the emulator foundation status. The memory component and its unit test are the first implementation milestone.

## Legal note
Wins2 does not distribute PlayStation 2 BIOS files or game images. Use files obtained and dumped lawfully from hardware and media you own.

## Progress policy
A feature is marked implemented only when code exists. It is marked validated only after the corresponding build and tests pass. Game compatibility is tracked per title and configuration.
