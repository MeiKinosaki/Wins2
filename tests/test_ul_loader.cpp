#include "disc/ul_loader.hpp"
#include "disc/iso9660.hpp"
#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
int main(){
 const auto root=std::filesystem::temp_directory_path()/"wins2_ul_test";std::error_code ec;std::filesystem::remove_all(root,ec);std::filesystem::create_directories(root);
 std::array<unsigned char,64> rec{};const char title[]="Test OPL Game";std::copy(title,title+sizeof(title)-1,rec.begin());
 rec[32]='u';rec[33]='l';rec[34]='.';const char id[]="SLUS_123.45";std::copy(id,id+sizeof(id)-1,rec.begin()+35);rec[47]=1;rec[48]=0x14;rec[53]=0x08;
 {std::ofstream cfg(root/"ul.cfg",std::ios::binary);cfg.write(reinterpret_cast<const char*>(rec.data()),static_cast<std::streamsize>(rec.size()));}
 wins2::UlLoader loader;std::string error;if(!loader.scan(root,&error)){std::cerr<<error<<"\n";return 1;}
 if(loader.games().size()!=1){std::cerr<<"Expected one UL record, got "<<loader.games().size()<<"\\n";return 2;}
 if(loader.games()[0].name!="Test OPL Game"||loader.games()[0].startup!="SLUS_123.45"||loader.games()[0].parts!=1||loader.games()[0].media!=0x14){std::cerr<<"UL record fields mismatch: "<<loader.games()[0].name<<" / "<<loader.games()[0].startup<<" / "<<unsigned(loader.games()[0].parts)<<" / "<<unsigned(loader.games()[0].media)<<"\\n";return 2;}
 const auto part=loader.partPath(loader.games()[0],0);
 if(part.filename().string()!="ul.AF9FCC67.SLUS_123.45.00") {std::cerr<<part.filename().string()<<"\n";return 3;}
 {std::ofstream out(part,std::ios::binary);out<<"ULDATA";}
 std::array<char,6> data{};if(!loader.read(loader.games()[0],0,data.data(),data.size(),&error)){std::cerr<<error<<"\n";return 4;}
 if(std::string(data.data(),data.size())!="ULDATA")return 5;
 if(loader.read(loader.games()[0],0,nullptr,1,&error))return 6;
 std::array<char,2048> sectorData{};sectorData.fill('U');{std::ofstream out(part,std::ios::binary);out.write(sectorData.data(),sectorData.size());}
 wins2::VirtualCdDvdDrive drive;if(!drive.mountUl(root,0,&error))return 7;
 std::array<char,2048> sectorRead{};if(!drive.readSector(0,sectorRead.data(),sectorRead.size(),&error)||sectorRead!=sectorData)return 8;
 if(drive.discInfo().serial!="SLUS_123.45")return 9;drive.eject();if(drive.mounted())return 10;
 std::array<char,7> tooLong{};if(loader.read(loader.games()[0],0,tooLong.data(),tooLong.size(),&error))return 11;
 if(loader.read(loader.games()[0],UINT64_MAX-2,tooLong.data(),4,&error))return 12;
 std::filesystem::remove(part,ec);if(loader.read(loader.games()[0],0,data.data(),data.size(),&error))return 13;
 std::filesystem::remove_all(root,ec);std::cout<<"Wins2 OPL UL metadata, CRC and chunk-reader tests passed.\n";return 0;
}
