#include "core/memory.hpp"

#include <cstdint>
#include <iostream>

int main() {
    wins2::Memory memory;
    std::uint32_t value = 0;

    if (memory.ramSize() != wins2::Memory::kEeRamSize) return 1;
    if (!memory.write32(0x100u, 0x78563412u)) return 2;
    if (!memory.read32(0x100u, value) || value != 0x78563412u) return 3;
    if (!memory.read32(0x80000100u, value) || value != 0x78563412u) return 4;
    if (!memory.write8(0xA0000100u, 0xABu)) return 5;
    std::uint8_t byte = 0;
    if (!memory.read8(0x100u, byte) || byte != 0xABu) return 6;
    if (memory.write32(0x101u, 0u)) return 7; // unaligned access
    if (memory.read32(0x02000000u, value)) return 8; // first byte past RAM

    std::cout << "Wins2 memory tests passed.\\n";
    return 0;
}
