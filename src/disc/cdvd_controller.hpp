#pragma once
#include "disc/iso9660.hpp"
#include <cstdint>
#include <string>
#include <vector>
namespace wins2 {
enum class CdvdStatus : std::uint8_t { NoDisc, Ready, Reading, Error };
enum class CdvdCommand : std::uint8_t { GetDiscInfo=0x01, ReadSectors=0x02, Seek=0x03 };
struct CdvdCommandResult {
 std::uint8_t command{0};
 CdvdStatus status{CdvdStatus::NoDisc};
 std::uint32_t lba{0};
 std::uint32_t sectors{0};
 std::uint32_t transferredBytes{0};
 bool success{false};
};
class CdvdController {
public:
 explicit CdvdController(VirtualCdDvdDrive& drive) noexcept:drive_(drive){}
 [[nodiscard]] bool refresh(std::string* error=nullptr);
 [[nodiscard]] bool read(std::uint32_t lba,std::uint32_t sectors,std::string* error=nullptr);
 // Host-side command shim; this is not yet the PS2 hardware register protocol.
 [[nodiscard]] bool execute(CdvdCommand command,std::uint32_t lba=0,std::uint32_t sectors=0,std::string* error=nullptr);
 [[nodiscard]] const CdvdCommandResult& lastCommand()const noexcept{return lastCommand_;}
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
 CdvdCommandResult lastCommand_{};
};
}
