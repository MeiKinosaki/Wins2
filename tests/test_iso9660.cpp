#include "disc/iso9660.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
namespace {
constexpr std::size_t S=2048;
void le16(std::uint8_t* p,std::uint16_t v){p[0]=static_cast<std::uint8_t>(v);p[1]=static_cast<std::uint8_t>(v>>8u);}
void be16(std::uint8_t* p,std::uint16_t v){p[0]=static_cast<std::uint8_t>(v>>8u);p[1]=static_cast<std::uint8_t>(v);}
void le32(std::uint8_t* p,std::uint32_t v){for(unsigned i=0;i<4;++i)p[i]=static_cast<std::uint8_t>(v>>(8u*i));}
void be32(std::uint8_t* p,std::uint32_t v){for(unsigned i=0;i<4;++i)p[i]=static_cast<std::uint8_t>(v>>(8u*(3u-i)));}
std::size_t record(std::uint8_t* p,std::uint32_t lba,std::uint32_t size,std::uint8_t flags,const std::string& name){
 const auto n=name.size();const auto len=static_cast<std::uint8_t>(33+n+((n%2)==0?1:0));p[0]=len;p[1]=0;le32(p+2,lba);be32(p+6,lba);le32(p+10,size);be32(p+14,size);p[25]=flags;le16(p+28,1);be16(p+30,1);p[32]=static_cast<std::uint8_t>(n);std::copy(name.begin(),name.end(),p+33);return len;
}
}
int main(){
 const auto path=std::filesystem::temp_directory_path()/"wins2_synthetic_ps2.iso";
 std::vector<std::uint8_t> image(24*S,0);
 auto* pvd=image.data()+16*S;pvd[0]=1;std::copy_n("CD001",5,pvd+1);pvd[6]=1;
 std::copy_n("WINS2 TEST DISC",15,pvd+40);le32(pvd+80,24);be32(pvd+84,24);le16(pvd+128,2048);be16(pvd+130,2048);
 record(pvd+156,20,S,2,std::string(1,'\0'));
 auto* dir=image.data()+20*S;std::size_t pos=0;pos+=record(dir+pos,20,S,2,std::string(1,'\0'));pos+=record(dir+pos,20,S,2,std::string(1,'\1'));pos+=record(dir+pos,21,64,0,"SYSTEM.CNF;1");
 const std::string cnf="BOOT2 = cdrom0:\\SLUS_123.45;1\n";std::copy(cnf.begin(),cnf.end(),image.begin()+21*S);
 {std::ofstream out(path,std::ios::binary);out.write(reinterpret_cast<const char*>(image.data()),static_cast<std::streamsize>(image.size()));}
 wins2::VirtualCdDvdDrive drive;std::string error;if(!drive.mountIso(path,&error)){std::cerr<<error<<"\n";return 1;}
 if(!drive.mounted()||drive.discInfo().volumeId.empty()||drive.discInfo().serial!="SLUS_123.45")return 2;
 std::array<std::uint8_t,S> sector{};if(!drive.readSector(21,sector.data(),sector.size(),&error))return 3;
 if(std::string(reinterpret_cast<char*>(sector.data()),8)!="BOOT2 = ")return 4;
 if(drive.readSector(24,sector.data(),sector.size(),&error))return 5;
 if(drive.readSector(0,sector.data(),512,&error))return 6;
 drive.eject();if(drive.mounted()||drive.readSector(0,sector.data(),sector.size(),&error))return 7;
 std::filesystem::remove(path);std::cout<<"ISO 9660 sector, SYSTEM.CNF PS2 serial and virtual drive tests passed.\n";return 0;
}
