#pragma once
#include "core/iop_memory.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
namespace wins2 {
enum class IopException : std::uint8_t { Interrupt=0, AddressLoad=4, AddressStore=5, Syscall=8, Breakpoint=9, ReservedInstruction=10, CoprocessorUnusable=11, Overflow=12 };
struct R3000AState {
 std::array<std::uint32_t,32> gpr{};
 std::array<std::uint32_t,16> cop0{};
 std::uint32_t hi{0},lo{0},pc{0},lastInstruction{0},branchTarget{0},branchPc{0},badVaddr{0};
 bool branchPending{false},halted{false};
};
class R3000A {
public:
 static constexpr std::size_t kRegisterCount=32,kCop0RegisterCount=16;
 static constexpr std::uint32_t kGeneralExceptionVector=0x80000080u,kBootstrapExceptionVector=0xBFC00180u;
 static constexpr std::uint32_t kStatusIEc=1u,kStatusInterruptMask=0x0000FF00u,kStatusBEV=1u<<22u;
 static constexpr std::uint32_t kCauseBD=1u<<31u,kCauseIPMask=0x0000FF00u,kCauseExcCodeMask=0x0000007Cu;
 explicit R3000A(IopMemory& memory)noexcept;
 void reset(std::uint32_t pc=0)noexcept;
 [[nodiscard]] bool step()noexcept;
 [[nodiscard]] std::uint32_t reg(std::size_t index)const noexcept;
 void setReg(std::size_t index,std::uint32_t value)noexcept;
 [[nodiscard]] std::uint32_t cop0(std::size_t index)const noexcept;
 void setCop0(std::size_t index,std::uint32_t value)noexcept;
 void setInterruptLine(std::uint8_t line,bool asserted)noexcept;
 [[nodiscard]] bool interruptPending()const noexcept;
 [[nodiscard]] bool halted()const noexcept{return halted_;}
 [[nodiscard]] std::uint32_t pc()const noexcept{return pc_;}
 [[nodiscard]] std::uint32_t badVaddr()const noexcept{return bad_vaddr_;}
 [[nodiscard]] R3000AState captureState()const noexcept;
 void restoreState(const R3000AState& state)noexcept;
private:
 void scheduleBranch(std::uint32_t target,std::uint32_t branchPc)noexcept;
 void raiseException(IopException exception,std::uint32_t faultPc,bool inDelaySlot,std::uint32_t badAddress=0)noexcept;
 void updateCausePending()noexcept;
 IopMemory& memory_;
 std::array<std::uint32_t,kRegisterCount> gpr_{};
 std::array<std::uint32_t,kCop0RegisterCount> cop0_{};
 std::uint32_t hi_{0},lo_{0},pc_{0},last_instruction_{0},branch_target_{0},branch_pc_{0},bad_vaddr_{0};
 bool branch_pending_{false},halted_{false};
};
}
