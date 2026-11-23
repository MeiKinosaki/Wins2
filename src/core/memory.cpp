#include "core/memory.hpp"

namespace wins2 {

Memory::Memory() : ram_(kEeRamSize, 0) {}

bool Memory::translateRamAddress(const std::uint32_t address,
                                  std::size_t& offset) noexcept {
    // Physical EE RAM is exposed at 0x00000000; KSEG0/KSEG1 aliases map
    // to the low physical 29-bit address space.
    std::uint32_t physical = address;
    if ((address & 0xE0000000u) == 0x80000000u ||
        (address & 0xE0000000u) == 0xA0000000u) {
        physical = address & 0x1FFFFFFFu;
    }
    if (physical >= kEeRamSize) {
        return false;
    }
    offset = static_cast<std::size_t>(physical);
    return true;
}

bool Memory::read8(const std::uint32_t address, std::uint8_t& value) const noexcept {
    std::size_t offset = 0;
    if (!translateRamAddress(address, offset)) return false;
    value = ram_[offset];
    return true;
}

bool Memory::read32(const std::uint32_t address, std::uint32_t& value) const noexcept {
    if ((address & 3u) != 0u) return false;
    std::size_t offset = 0;
    if (!translateRamAddress(address, offset) || offset + 4 > ram_.size()) return false;
    value = static_cast<std::uint32_t>(ram_[offset]) |
            (static_cast<std::uint32_t>(ram_[offset + 1]) << 8u) |
            (static_cast<std::uint32_t>(ram_[offset + 2]) << 16u) |
            (static_cast<std::uint32_t>(ram_[offset + 3]) << 24u);
    return true;
}

bool Memory::write8(const std::uint32_t address, const std::uint8_t value) noexcept {
    std::size_t offset = 0;
    if (!translateRamAddress(address, offset)) return false;
    ram_[offset] = value;
    return true;
}

bool Memory::write32(const std::uint32_t address, const std::uint32_t value) noexcept {
    if ((address & 3u) != 0u) return false;
    std::size_t offset = 0;
    if (!translateRamAddress(address, offset) || offset + 4 > ram_.size()) return false;
    for (unsigned i = 0; i < 4; ++i) {
        ram_[offset + i] = static_cast<std::uint8_t>((value >> (i * 8u)) & 0xFFu);
    }
    return true;
}

} // namespace wins2
