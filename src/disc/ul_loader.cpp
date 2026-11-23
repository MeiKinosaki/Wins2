#include "disc/ul_loader.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <fstream>
#include <utility>
namespace wins2 {
namespace {
void err(std::string* e,const std::string& s){if(e)*e=s;}
std::string fixedText(const unsigned char* p,std::size_t n){
 std::size_t len=0;while(len<n&&p[len]!=0)++len;return std::string(reinterpret_cast<const char*>(p),len);
}
// Matches the OPL/USBExtreme CRC table and includes the terminating NUL byte.
std::uint32_t oplCrc32(const std::string& name){
 std::array<std::uint32_t,256> table{};
 for(std::uint32_t t=0;t<256;++t){
  std::uint32_t value=t<<24u;
  for(unsigned bit=0;bit<8;++bit)
   value=(value&0x80000000u)?(value<<1u):((value<<1u)^0x04C11DB7u);
  table[255u-t]=value;
 }
 std::uint32_t crc=0;
 for(std::size_t i=0;i<=name.size();++i){
  const auto byte=(i==name.size())?0u:static_cast<unsigned char>(name[i]);
  crc=table[byte^((crc>>24u)&0xffu)]^((crc<<8u)&0xffffff00u);
 }
 return crc;
}
}
bool UlLoader::scan(const std::filesystem::path& root,std::string* error){
 root_=root;games_.clear();std::ifstream in(root_/"ul.cfg",std::ios::binary);
 if(!in){err(error,"ul.cfg was not found in the selected OPL/USBExtreme folder.");return false;}
 in.seekg(0,std::ios::end);const auto length=in.tellg();in.seekg(0);
 if(length<0||static_cast<std::uint64_t>(length)%kRecordSize!=0){err(error,"ul.cfg has an invalid size; expected complete 64-byte records.");return false;}
 const auto count=static_cast<std::size_t>(length)/kRecordSize;
 for(std::size_t i=0;i<count;++i){
  std::array<unsigned char,kRecordSize> r{};in.read(reinterpret_cast<char*>(r.data()),static_cast<std::streamsize>(r.size()));
  if(!in){err(error,"Could not read a complete ul.cfg record.");games_.clear();return false;}
  if(r[32]!='u'||r[33]!='l'||r[34]!='.'||r[53]!=0x08||r[47]==0)continue;
  UlGame g;g.name=fixedText(r.data(),32);g.startup=fixedText(r.data()+35,12);g.parts=r[47];g.media=r[48];
  if(g.name.empty()||g.startup.empty()||(g.media!=0x12&&g.media!=0x14))continue;
  g.nameCrc=oplCrc32(g.name);games_.push_back(std::move(g));
 }
 return true;
}
std::filesystem::path UlLoader::partPath(const UlGame& game,std::size_t part)const{
 if(part>=game.parts)return {};
 char name[96];std::snprintf(name,sizeof(name),"ul.%08X.%s.%02x",game.nameCrc,game.startup.c_str(),static_cast<unsigned>(part));
 return root_/name;
}
bool UlLoader::read(const UlGame& game,std::uint64_t offset,void* destination,std::size_t bytes,std::string* error)const{
 if(bytes&&destination==nullptr){err(error,"Destination buffer is null.");return false;}
 auto* out=static_cast<char*>(destination);std::size_t remaining=bytes;
 while(remaining){
  const auto part=static_cast<std::size_t>(offset/kChunkSize);const auto inPart=static_cast<std::size_t>(offset%kChunkSize);
  if(part>=game.parts){err(error,"Read exceeds the end of the USBExtreme game.");return false;}
  const auto path=partPath(game,part);std::ifstream file(path,std::ios::binary);
  if(!file){err(error,"A required USBExtreme part is missing: "+path.string());return false;}
  file.seekg(0,std::ios::end);const auto end=file.tellg();
  if(end<0||inPart>=static_cast<std::uint64_t>(end)){err(error,"USBExtreme part is shorter than the requested offset.");return false;}
  const auto available=static_cast<std::size_t>(static_cast<std::uint64_t>(end)-inPart);
  const auto take=std::min(remaining,available);file.seekg(static_cast<std::streamoff>(inPart));
  file.read(out,static_cast<std::streamsize>(take));if(!file){err(error,"Failed reading USBExtreme chunk.");return false;}
  out+=take;remaining-=take;offset+=take;
 }
 return true;
}
}
