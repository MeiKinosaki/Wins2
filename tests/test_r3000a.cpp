#include "core/iop_memory.hpp"
#include "cpu/iop/r3000a.hpp"
#include <cstdint>
int main(){
 wins2::IopMemory memory;wins2::R3000A cpu(memory);
 // addiu t0,zero,7; addiu t1,zero,9; addu t2,t0,t1; sw t2,0x100(zero); lw t3,0x100(zero)
 const std::uint32_t program[]={0x24080007u,0x24090009u,0x01095021u,0xAC0A0100u,0x8C0B0100u};
 for(std::size_t i=0;i<5;++i)if(!memory.write32(static_cast<std::uint32_t>(i*4),program[i]))return 1;
 for(int i=0;i<5;++i)if(!cpu.step())return 2;
 if(cpu.reg(8)!=7||cpu.reg(9)!=9||cpu.reg(10)!=16||cpu.reg(11)!=16)return 3;
 if(cpu.reg(0)!=0||cpu.pc()!=20)return 4;
 const auto state=cpu.captureState();cpu.setReg(11,99);cpu.restoreState(state);if(cpu.reg(11)!=16)return 5;
 cpu.reset(0x00200000u);if(cpu.step()||!cpu.halted())return 6;
 return 0;
}
