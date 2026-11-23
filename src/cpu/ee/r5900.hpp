#pragma once
#include "core/memory.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
namespace wins2 {
class R5900 {
public:
 static constexpr std::size_t kRegisterCount=32;
 explicit R5900(Memory& memory) noexcept;
 void reset(std::uint32_t pc=0) noexcept;
 [[nodiscard]] bool step() noexcept;
 [[nodiscard]] std::uint64_t reg(std::size_t index) const noexcept;
 void setReg(std::size_t index,std::uint64_t value) noexcept;
 [[nodiscard]] std::uint64_t hi() const noexcept{return hi_;}
 [[nodiscard]] std::uint64_t lo() const noexcept{return lo_;}
 [[nodiscard]] std::uint32_t pc() const noexcept{return pc_;}
 void setPc(std::uint32_t value) noexcept{pc_=value;}
 [[nodiscard]] bool halted() const noexcept{return halted_;}
 [[nodiscard]] std::uint32_t lastInstruction() const noexcept{return last_instruction_;}
private:
 [[nodiscard]] static std::uint64_t signExtend32(std::uint32_t value) noexcept;
 [[nodiscard]] static std::int64_t signExtend16(std::uint16_t value) noexcept;
 void scheduleBranch(std::uint32_t target) noexcept;
 Memory& memory_;
 std::array<std::uint64_t,kRegisterCount> gpr_{};
 std::uint64_t hi_{0},lo_{0};
 std::uint32_t pc_{0},last_instruction_{0},branch_target_{0};
 bool branch_pending_{false},halted_{false};
};
}
