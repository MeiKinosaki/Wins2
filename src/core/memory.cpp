#include "core/memory.hpp"
namespace wins2 {
Memory::Memory():ram_(kEeRamSize,0){}
bool Memory::translateRamAddress(std::uint32_t address,std::size_t& offset) noexcept{
 std::uint32_t physical=address;
 if((address&0xE0000000u)==0x80000000u||(address&0xE0000000u)==0xA0000000u)physical=address&0x1FFFFFFFu;
 if(physical>=kEeRamSize)return false;offset=static_cast<std::size_t>(physical);return true;
}
bool Memory::read8(std::uint32_t a,std::uint8_t& v)const noexcept{std::size_t o=0;if(!translateRamAddress(a,o))return false;v=ram_[o];return true;}
bool Memory::read32(std::uint32_t a,std::uint32_t& v)const noexcept{
 if(a&3u)return false;std::size_t o=0;if(!translateRamAddress(a,o)||o+4>ram_.size())return false;
 v=static_cast<std::uint32_t>(ram_[o])|(static_cast<std::uint32_t>(ram_[o+1])<<8u)|(static_cast<std::uint32_t>(ram_[o+2])<<16u)|(static_cast<std::uint32_t>(ram_[o+3])<<24u);return true;
}
bool Memory::write8(std::uint32_t a,std::uint8_t v)noexcept{std::size_t o=0;if(!translateRamAddress(a,o))return false;ram_[o]=v;return true;}
bool Memory::write32(std::uint32_t a,std::uint32_t v)noexcept{
 if(a&3u)return false;std::size_t o=0;if(!translateRamAddress(a,o)||o+4>ram_.size())return false;
 for(unsigned i=0;i<4;++i)ram_[o+i]=static_cast<std::uint8_t>(v>>(i*8u));return true;
}
bool Memory::restoreRam(const std::vector<std::uint8_t>& data)noexcept{if(data.size()!=ram_.size())return false;ram_=data;return true;}
}
