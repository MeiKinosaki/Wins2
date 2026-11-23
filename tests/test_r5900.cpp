#include "core/memory.hpp"
#include "cpu/ee/r5900.hpp"
#include <cstdint>
#include <iostream>
namespace {
constexpr std::uint32_t it(std::uint32_t op,std::uint32_t rs,std::uint32_t rt,std::uint16_t im){return(op<<26u)|(rs<<21u)|(rt<<16u)|im;}
constexpr std::uint32_t rr(std::uint32_t rs,std::uint32_t rt,std::uint32_t rd,std::uint32_t sa,std::uint32_t fn){return(rs<<21u)|(rt<<16u)|(rd<<11u)|(sa<<6u)|fn;}
int fail(int n,const char* message){std::cerr<<"R5900 test "<<n<<" failed: "<<message<<"\\n";return n;}
}
int main(){
 wins2::Memory m; wins2::R5900 c(m);
 if(!m.write32(0,it(9,0,1,7))||!m.write32(4,it(9,0,2,5))||!m.write32(8,rr(1,2,3,0,0x21))||!m.write32(12,it(0x0D,3,4,0x10))||!m.write32(16,it(0x0F,0,5,0x1234)))return fail(1,"write arithmetic program");
 c.reset(); for(int i=0;i<5;++i)if(!c.step())return fail(2,"execute arithmetic program");
 if(c.reg(1)!=7||c.reg(2)!=5||c.reg(3)!=12||c.reg(4)!=28||c.reg(5)!=0x12340000u||c.reg(0)!=0)return fail(3,"arithmetic register results");
 if(!m.write32(0x100,rr(1,2,0,0,0x18))||!m.write32(0x104,rr(0,0,6,0,0x12)))return fail(4,"write multiply program");
 c.reset(0x100);c.setReg(1,7);c.setReg(2,5);
 if(!c.step()||c.lo()!=35||c.hi()!=0)return fail(5,"MULT HI/LO result");
 if(!c.step()||c.reg(6)!=35)return fail(6,"MFLO result");
 if(!m.write32(0x120,rr(1,2,7,0,0x2D))||!m.write32(0x124,rr(0,7,8,4,0x38)))return fail(7,"write 64-bit program");
 c.reset(0x120);c.setReg(1,0x100000000ull);c.setReg(2,3);
 if(!c.step()||c.reg(7)!=0x100000003ull)return fail(8,"DADDU result");
 if(!c.step()||c.reg(8)!=0x1000000030ull)return fail(9,"DSLL result");
 if(!m.write32(0x40,it(4,0,0,3))||!m.write32(0x44,it(9,0,6,9)))return fail(10,"write branch program");
 c.reset(0x40);
 if(!c.step()||c.pc()!=0x44)return fail(11,"branch instruction PC");
 if(!c.step()||c.reg(6)!=9||c.pc()!=0x50)return fail(12,"branch delay slot and target");
 if(!m.write32(0x80,0xFC000000u))return fail(13,"write unsupported instruction");
 c.reset(0x80);
 if(c.step()||!c.halted())return fail(14,"unsupported instruction must halt");
 std::cout<<"Wins2 R5900 tests passed.\\n";return 0;
}
