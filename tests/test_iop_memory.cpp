#include "core/iop_memory.hpp"
#include <cstdint>
#include <iostream>
int main(){
 wins2::IopMemory memory;std::uint32_t word=0;std::uint8_t byte=0;
 if(memory.ramSize()!=2u*1024u*1024u)return 1;
 if(!memory.write32(0x100u,0x78563412u))return 2;
 if(!memory.read32(0x100u,word)||word!=0x78563412u)return 3;
 if(!memory.read32(0x80000100u,word)||word!=0x78563412u)return 4;
 if(!memory.write8(0xA0000100u,0xABu))return 5;
 if(!memory.read8(0x100u,byte)||byte!=0xABu)return 6;
 if(!memory.write32(wins2::IopMemory::kScratchpadBase,0xA1B2C3D4u))return 7;
 if(!memory.read32(wins2::IopMemory::kScratchpadBase,word)||word!=0xA1B2C3D4u)return 8;
 if(!memory.write32(wins2::IopMemory::kScratchpadBase+1020u,0x12345678u))return 9;
 if(!memory.read32(wins2::IopMemory::kScratchpadBase+1020u,word)||word!=0x12345678u)return 10;
 if(memory.read32(wins2::IopMemory::kScratchpadBase+1021u,word))return 14;
 if(memory.read8(0x00200000u,byte))return 11;
 if(memory.write32(0x101u,0u))return 12;
 if(memory.read32(0x1F8003FEu,word))return 13;
 std::cout<<"Wins2 IOP memory tests passed.\n";return 0;
}
