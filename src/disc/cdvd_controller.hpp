#pragma once
#include "disc/iso9660.hpp"
#include <cstdint>
#include <string>
#include <vector>
namespace wins2 {
enum class CdvdStatus : std::uint8_t { NoDisc, Ready, Reading, Error };
class CdvdController {
public:
 explicit CdvdController(VirtualCdDvdDrive& drive) noexcept:drive_(drive){}
 [[nodiscard]] bool refresh(std::string* error=nullptr);
 [[nodiscard]] bool read(std::uint32_t lba,std::uint32_t sectors,std::string* error=nullptr);
 [[nodiscard]] const std::vector<std::uint8_t>& data()const noexcept{return buffer_;}
 [[nodiscard]] CdvdStatus status()const noexcept{return status_;}
 [[nodiscard]] std::uint32_t lastLba()const noexcept{return lastLba_;}
 [[nodiscard]] const IsoDiscInfo& discInfo()const noexcept{return drive_.discInfo();}
 void reset() noexcept;
private:
 VirtualCdDvdDrive& drive_;
 std::vector<std::uint8_t> buffer_;
 CdvdStatus status_{CdvdStatus::NoDisc};
 std::uint32_t lastLba_{0};
};
}
