#include "core/iop_memory.hpp"
namespace wins2 {
IopMemory::IopMemory():ram_(kRamSize,0),scratchpad_(kScratchpadSize,0){}
bool IopMemory::translateRam(std::uint32_t address,std::size_t& offset)noexcept{
 std::uint32_t physical=address;
 if((address&0xE0000000u)==0x80000000u||(address&0xE0000000u)==0xA0000000u)physical=address&0x1FFFFFFFu;
 if(physical>=kRamSize)return false;
 offset=static_cast<std::size_t>(physical);return true;
}
bool IopMemory::translateScratchpad(std::uint32_t address,std::size_t& offset)noexcept{
 if(address<kScratchpadBase||static_cast<std::uint64_t>(address)-kScratchpadBase>=kScratchpadSize)return false;
 offset=static_cast<std::size_t>(address-kScratchpadBase);return true;
}
bool IopMemory::read8(std::uint32_t address,std::uint8_t& value)const noexcept{
 std::size_t offset=0;
 if(translateRam(address,offset)){value=ram_[offset];return true;}
 if(translateScratchpad(address,offset)){value=scratchpad_[offset];return true;}
 return false;
}
bool IopMemory::read32(std::uint32_t address,std::uint32_t& value)const noexcept{
 if(address&3u)return false;
 std::size_t offset=0;const std::vector<std::uint8_t>* region=nullptr;
 if(translateRam(address,offset))region=&ram_;
 else if(translateScratchpad(address,offset))region=&scratchpad_;
 else return false;
 if(offset+4>region->size())return false;
 value=static_cast<std::uint32_t>((*region)[offset])|(static_cast<std::uint32_t>((*region)[offset+1])<<8u)|(static_cast<std::uint32_t>((*region)[offset+2])<<16u)|(static_cast<std::uint32_t>((*region)[offset+3])<<24u);
 return true;
}
bool IopMemory::write8(std::uint32_t address,std::uint8_t value)noexcept{
 std::size_t offset=0;
 if(translateRam(address,offset)){ram_[offset]=value;return true;}
 if(translateScratchpad(address,offset)){scratchpad_[offset]=value;return true;}
 return false;
}
bool IopMemory::write32(std::uint32_t address,std::uint32_t value)noexcept{
 if(address&3u)return false;
 std::size_t offset=0;std::vector<std::uint8_t>* region=nullptr;
 if(translateRam(address,offset))region=&ram_;
 else if(translateScratchpad(address,offset))region=&scratchpad_;
 else return false;
 if(offset+4>region->size())return false;
 for(unsigned i=0;i<4;++i)(*region)[offset+i]=static_cast<std::uint8_t>(value>>(i*8u));
 return true;
}
}
