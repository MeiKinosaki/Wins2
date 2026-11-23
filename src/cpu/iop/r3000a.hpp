#pragma once
#include "core/iop_memory.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
namespace wins2 {
struct R3000AState {
 std::array<std::uint32_t,32> gpr{};
 std::uint32_t hi{0},lo{0},pc{0},lastInstruction{0},branchTarget{0};
 bool branchPending{false},halted{false};
};
class R3000A {
public:
 static constexpr std::size_t kRegisterCount=32;
 explicit R3000A(IopMemory& memory)noexcept;
 void reset(std::uint32_t pc=0)noexcept;
 [[nodiscard]] bool step()noexcept;
 [[nodiscard]] std::uint32_t reg(std::size_t index)const noexcept;
 void setReg(std::size_t index,std::uint32_t value)noexcept;
 [[nodiscard]] std::uint32_t pc()const noexcept{return pc_;}
 [[nodiscard]] bool halted()const noexcept{return halted_;}
 [[nodiscard]] R3000AState captureState()const noexcept;
 void restoreState(const R3000AState& state)noexcept;
private:
 void scheduleBranch(std::uint32_t target)noexcept;
 IopMemory& memory_;
 std::array<std::uint32_t,kRegisterCount> gpr_{};
 std::uint32_t hi_{0},lo_{0},pc_{0},last_instruction_{0},branch_target_{0};
 bool branch_pending_{false},halted_{false};
};
}
