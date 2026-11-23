#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
namespace wins2 {
class IopMemory {
public:
 static constexpr std::size_t kRamSize=2u*1024u*1024u;
 static constexpr std::uint32_t kScratchpadBase=0x1F800000u;
 static constexpr std::size_t kScratchpadSize=1024u;
 IopMemory();
 [[nodiscard]] bool read8(std::uint32_t address,std::uint8_t& value)const noexcept;
 [[nodiscard]] bool read32(std::uint32_t address,std::uint32_t& value)const noexcept;
 [[nodiscard]] bool write8(std::uint32_t address,std::uint8_t value)noexcept;
 [[nodiscard]] bool write32(std::uint32_t address,std::uint32_t value)noexcept;
 [[nodiscard]] std::size_t ramSize()const noexcept{return ram_.size();}
 [[nodiscard]] const std::vector<std::uint8_t>& ramData()const noexcept{return ram_;}
private:
 [[nodiscard]] static bool translateRam(std::uint32_t address,std::size_t& offset)noexcept;
 [[nodiscard]] static bool translateScratchpad(std::uint32_t address,std::size_t& offset)noexcept;
 std::vector<std::uint8_t> ram_;
 std::vector<std::uint8_t> scratchpad_;
};
}
