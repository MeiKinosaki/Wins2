#include "core/iop_memory.hpp"
#include "cpu/iop/r3000a.hpp"
#include <cstdint>
#include <cstdio>
#define FAIL(n) do { std::printf("R3000A test failed at checkpoint %d\\n", n); return n; } while(false)
namespace {
constexpr std::uint32_t mfc0(unsigned rt,unsigned rd){return (0x10u<<26u)|(rt<<16u)|(rd<<11u);}
constexpr std::uint32_t mtc0(unsigned rt,unsigned rd){return (0x10u<<26u)|(4u<<21u)|(rt<<16u)|(rd<<11u);}
}
int main(){
 wins2::IopMemory memory;wins2::R3000A cpu(memory);
 // COP0 transfer round-trip: write Status, read it back.
 memory.write32(0,mtc0(8,12));memory.write32(4,mfc0(9,12));cpu.setReg(8,0x0000FF01u);
 if(!cpu.step()||cpu.cop0(12)!=0x0000FF01u)FAIL(1);
 if(!cpu.step()||cpu.reg(9)!=0x0000FF01u)FAIL(2);
 // IP2-IP7 are hardware lines. Multiple pending lines are retained; software decides service priority.
 cpu.reset(0x100u);cpu.setCop0(12,0x00000201u);cpu.setInterruptLine(5,true);cpu.setInterruptLine(2,true);
 if(cpu.interruptPending())FAIL(3);
 cpu.setCop0(12,0x00001001u);if(!cpu.interruptPending())FAIL(4);
 if(!cpu.step()||cpu.pc()!=wins2::R3000A::kGeneralExceptionVector)FAIL(5);
 if(((cpu.cop0(13)&wins2::R3000A::kCauseExcCodeMask)>>2u)!=0u)FAIL(6);
 if((cpu.cop0(13)&wins2::R3000A::kCauseIPMask)!=(1u<<12u|1u<<15u))FAIL(7);
 if(cpu.cop0(14)!=0x100u)FAIL(8);
 // RFE restores the previous interrupt-enable stack.
 cpu.setCop0(12,0x00000404u);memory.write32(wins2::R3000A::kGeneralExceptionVector,0x42000010u);
 if(!cpu.step()||(cpu.cop0(12)&0x0Fu)!=1u)FAIL(9);
 // SYSCALL exception captures EPC and vectors through BEV when requested.
 cpu.reset(0x200u);memory.write32(0x200u,0x0000000Cu);
 if(!cpu.step()||cpu.pc()!=wins2::R3000A::kGeneralExceptionVector||cpu.cop0(14)!=0x200u)FAIL(10);
 if(((cpu.cop0(13)&wins2::R3000A::kCauseExcCodeMask)>>2u)!=8u)FAIL(11);
 cpu.reset(0x204u);cpu.setCop0(12,wins2::R3000A::kStatusBEV);memory.write32(0x204u,0x0000000Du);
 if(!cpu.step()||cpu.pc()!=wins2::R3000A::kBootstrapExceptionVector)FAIL(12);
 // Exception in a branch delay slot records branch PC and BD.
 cpu.reset(0x300u);memory.write32(0x300u,0x10000001u);memory.write32(0x304u,0x0000000Cu);
 if(!cpu.step()||!cpu.step()||cpu.cop0(14)!=0x300u||(cpu.cop0(13)&wins2::R3000A::kCauseBD)==0)FAIL(13);
 // Reserved instruction raises RI instead of silently halting.
 cpu.reset(0x400u);memory.write32(0x400u,0xFFFFFFFFu);
 if(!cpu.step()||cpu.halted()||((cpu.cop0(13)&wins2::R3000A::kCauseExcCodeMask)>>2u)!=10u)FAIL(14);
 // Interrupt asserted between branch and delay slot records BD/EPC of branch.
 cpu.reset(0x500u);cpu.setCop0(12,0x00001001u);memory.write32(0x500u,0x10000001u);memory.write32(0x504u,0);
 if(!cpu.step())FAIL(15);cpu.setInterruptLine(2,true);
 if(!cpu.step()||cpu.cop0(14)!=0x500u||(cpu.cop0(13)&wins2::R3000A::kCauseBD)==0)FAIL(16);
 // Signed ADD overflow raises exception code 12.
 cpu.reset(0x600u);cpu.setReg(1,0x7FFFFFFFu);cpu.setReg(2,1);memory.write32(0x600u,(1u<<21u)|(2u<<16u)|(3u<<11u)|0x20u);
 if(!cpu.step()||((cpu.cop0(13)&wins2::R3000A::kCauseExcCodeMask)>>2u)!=12u||cpu.cop0(14)!=0x600u)FAIL(17);
 // Save state preserves COP0 state.
 const auto state=cpu.captureState();cpu.setCop0(12,0);cpu.restoreState(state);if(cpu.cop0(12)!=state.cop0[12])FAIL(18);
 // Instruction-fetch fault in a delay slot must report the branch PC and set BD.
 cpu.reset(0x001FFFFCu);memory.write32(0x001FFFFCu,0x10000001u);
 if(!cpu.step())FAIL(19);
 if(!cpu.step()||cpu.cop0(14)!=0x001FFFFCu||(cpu.cop0(13)&wins2::R3000A::kCauseBD)==0)FAIL(20);
 if(((cpu.cop0(13)&wins2::R3000A::kCauseExcCodeMask)>>2u)!=4u||cpu.cop0(8)!=0x00200000u)FAIL(21);
 // Misaligned LW must raise AdEL and preserve the exact faulting virtual address.
 cpu.reset(0x700u);cpu.setReg(1,0x100u);memory.write32(0x700u,(0x23u<<26u)|(1u<<21u)|(2u<<16u)|1u);
 if(!cpu.step()||((cpu.cop0(13)&wins2::R3000A::kCauseExcCodeMask)>>2u)!=4u||cpu.cop0(8)!=0x101u||cpu.cop0(14)!=0x700u)FAIL(22);
 // Misaligned SW must raise AdES without modifying memory.
 cpu.reset(0x720u);cpu.setReg(1,0x100u);cpu.setReg(2,0xAABBCCDDu);memory.write32(0x100u,0x11223344u);
 memory.write32(0x720u,(0x2Bu<<26u)|(1u<<21u)|(2u<<16u)|2u);
 if(!cpu.step()||((cpu.cop0(13)&wins2::R3000A::kCauseExcCodeMask)>>2u)!=5u||cpu.cop0(8)!=0x102u||cpu.cop0(14)!=0x720u)FAIL(23);
 std::uint32_t unchanged=0;if(!memory.read32(0x100u,unchanged)||unchanged!=0x11223344u)FAIL(24);
 // Software interrupt pending bits are writable, but masked requests do not fire.
 cpu.reset(0x740u);cpu.setCop0(13,0x00000300u);cpu.setCop0(12,0x00000001u);
 if(cpu.interruptPending())FAIL(25);
 cpu.setCop0(12,0x00000301u);if(!cpu.interruptPending())FAIL(26);
 return 0;
}
