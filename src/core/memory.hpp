#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace wins2 {

class Memory {
public:
    static constexpr std::size_t kEeRamSize = 32u * 1024u * 1024u;
    static constexpr std::size_t kScratchpadSize = 16u * 1024u;

    Memory();

    [[nodiscard]] bool read8(std::uint32_t address, std::uint8_t& value) const noexcept;
    [[nodiscard]] bool read32(std::uint32_t address, std::uint32_t& value) const noexcept;
    [[nodiscard]] bool write8(std::uint32_t address, std::uint8_t value) noexcept;
    [[nodiscard]] bool write32(std::uint32_t address, std::uint32_t value) noexcept;

    [[nodiscard]] std::size_t ramSize() const noexcept { return ram_.size(); }

private:
    [[nodiscard]] static bool translateRamAddress(std::uint32_t address,
                                                  std::size_t& offset) noexcept;
    std::vector<std::uint8_t> ram_;
};

} // namespace wins2
