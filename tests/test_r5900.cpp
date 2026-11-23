#include "core/memory.hpp"
#include "cpu/ee/r5900.hpp"
#include <cstdint>
#include <iostream>
namespace {
constexpr std::uint32_t it(std::uint32_t op,std::uint32_t rs,std::uint32_t rt,std::uint16_t im){return(op<<26u)|(rs<<21u)|(rt<<16u)|im;}
constexpr std::uint32_t rr(std::uint32_t rs,std::uint32_t rt,std::uint32_t rd,std::uint32_t sa,std::uint32_t fn){return(rs<<21u)|(rt<<16u)|(rd<<11u)|(sa<<6u)|fn;}
}
int main(){
 wins2::Memory m;wins2::R5900 c(m);
 if(!m.write32(0,it(9,0,1,7))||!m.write32(4,it(9,0,2,5))||!m.write32(8,rr(1,2,3,0,0x21))||!m.write32(12,it(0x0D,3,4,0x10))||!m.write32(16,it(0x0F,0,5,0x1234)))return 1;
 c.reset();for(int i=0;i<5;++i)if(!c.step())return 2;
 if(c.reg(1)!=7||c.reg(2)!=5||c.reg(3)!=12||c.reg(4)!=28||c.reg(5)!=0x12340000u||c.reg(0)!=0)return 3;
 if(!m.write32(0x40,it(4,0,0,3))||!m.write32(0x44,it(9,0,6,9))||!m.write32(0x48,it(9,0,7,1)))return 4;
 c.reset(0x40);if(!c.step()||c.pc()!=0x44)return 5;if(!c.step()||c.reg(6)!=9||c.pc()!=0x50)return 6;
 if(!m.write32(0x80,0xFC000000u))return 7;c.reset(0x80);if(c.step()||!c.halted())return 8;
 std::cout<<"Wins2 R5900 tests passed.\n";return 0;
}
