# Wins2

**PlayStation 2 emulator for Windows x64**, designed for low-memory systems (4 GB target) without mandatory AVX/AVX2.

> Status: early development. This is not yet a playable emulator. No game compatibility claims are made until tested.

## Goals
- Portable Windows 10/11 x64 distribution, no installer.
- Baseline x86-64/SSE2; AVX/AVX2 optional only.
- ISO, BIN/CUE, CHD and game-folder support (planned).
- Multi-disc entries and disc switching without restarting (planned).
- Game-specific graphics, speed and memory settings (planned).
- Save states, memory cards, library covers and controller configuration (planned).

## Portable directory layout
At startup Wins2 creates these folders beside the executable:
- `games/` — game images and folders.
- `covers/` — user-provided game cover artwork.
- `saves/` — memory-card and per-game save data.
- `states/<game-id>/` — save-state slots.
- `bios/` — BIOS files supplied by the user.
- `screenshots/`, `config/`, `shaders/`, `cache/`, `logs/`.

The current save-state prototype stores EE RAM and the currently implemented R5900 CPU state in a versioned binary file with a checksum. It is not yet a complete PS2 save state: IOP, VU0/VU1, GS, SPU2, DMA, timers, CD/DVD and other devices must be included before compatibility across real game sessions can be claimed. Memory-card emulation is not implemented yet.

## Build
Requires CMake 3.20+ and a C++20 compiler.

```powershell
cmake -S . -B build -A x64 -DWINS2_BUILD_TESTS=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

## Architecture roadmap
- Emotion Engine (MIPS R5900), COP0, exceptions and FPU.
- IOP (MIPS R3000A), VU0/VU1, GS and SPU2.
- DMA, timers, interrupts, CD/DVD controller and BIOS interface.
- Memory cards, controller input, graphics/audio backends.
- Desktop library UI, per-game settings, multi-disc management.

## Legal
Wins2 does not distribute PS2 BIOS files or game images. Use files lawfully dumped from hardware and media you own.

## Progress policy
Implemented means code exists. Validated means corresponding builds and tests pass. Game compatibility is tracked per title and configuration.
