#include "disc/iso9660.hpp"
#include <algorithm>
#include <cctype>
#include <limits>

namespace wins2 {
namespace {
void setError(std::string* error,const std::string& message){if(error)*error=message;}
std::uint16_t le16(const std::uint8_t* p){return static_cast<std::uint16_t>(p[0]|(static_cast<unsigned>(p[1])<<8u));}
std::uint32_t le32(const std::uint8_t* p){return static_cast<std::uint32_t>(p[0])|(static_cast<std::uint32_t>(p[1])<<8u)|(static_cast<std::uint32_t>(p[2])<<16u)|(static_cast<std::uint32_t>(p[3])<<24u);}
std::uint32_t be32(const std::uint8_t* p){return (static_cast<std::uint32_t>(p[0])<<24u)|(static_cast<std::uint32_t>(p[1])<<16u)|(static_cast<std::uint32_t>(p[2])<<8u)|p[3];}
std::string trimField(const std::uint8_t* p,std::size_t n){std::size_t a=0,b=n;while(a<b&&p[a]==' ')++a;while(b>a&&(p[b-1]==' '||p[b-1]==0))--b;return std::string(reinterpret_cast<const char*>(p+a),b-a);}
std::string upper(std::string s){for(char& c:s)c=static_cast<char>(std::toupper(static_cast<unsigned char>(c)));return s;}
}
void Iso9660Image::close() noexcept{file_.close();file_.clear();fileSize_=0;rootLba_=rootSize_=0;info_={};}
bool Iso9660Image::open(const std::filesystem::path& path,std::string* error){
 file_.close();file_.clear();info_={};rootLba_=rootSize_=0;fileSize_=0;path_=path;
 file_.open(path_,std::ios::binary);if(!file_){setError(error,"Could not open ISO image.");return false;}
 file_.seekg(0,std::ios::end);const auto end=file_.tellg();if(end<0){setError(error,"Could not determine ISO image size.");file_.close();return false;}
 fileSize_=static_cast<std::uint64_t>(end);if(fileSize_<17ull*kSectorSize||fileSize_%kSectorSize){setError(error,"ISO image size is invalid or not aligned to 2048-byte sectors.");file_.close();return false;}
 std::array<std::uint8_t,kSectorSize> sector{};bool found=false;
 for(std::uint32_t lba=16;lba<80;++lba){
  file_.clear();file_.seekg(static_cast<std::streamoff>(static_cast<std::uint64_t>(lba)*kSectorSize));
  file_.read(reinterpret_cast<char*>(sector.data()),sector.size());if(!file_)break;
  if(sector[1]!='C'||sector[2]!='D'||sector[3]!='0'||sector[4]!='0'||sector[5]!='1')continue;
  if(sector[0]==1){
   const auto volumeLe=le32(sector.data()+80),volumeBe=be32(sector.data()+84);
   if(volumeLe==0||volumeLe!=volumeBe||static_cast<std::uint64_t>(volumeLe)*kSectorSize>fileSize_){setError(error,"ISO volume-space size is inconsistent.");file_.close();return false;}
   if(le16(sector.data()+128)!=kSectorSize||static_cast<std::uint16_t>((sector[130]<<8u)|sector[131])!=kSectorSize){setError(error,"Unsupported ISO logical block size.");file_.close();return false;}
   info_.volumeId=trimField(sector.data()+40,32);info_.volumeSectors=volumeLe;
   const auto* root=sector.data()+156;const auto len=root[0];
   if(len<34||156u+len>kSectorSize){setError(error,"Invalid ISO root directory record.");file_.close();return false;}
   rootLba_=le32(root+2);rootSize_=le32(root+10);
   if(rootLba_>=volumeLe||rootSize_==0||static_cast<std::uint64_t>(rootLba_)*kSectorSize+rootSize_>fileSize_){setError(error,"ISO root directory is outside the image.");file_.close();return false;}
   found=true;break;
  }
  if(sector[0]==255)break;
 }
 if(!found){setError(error,"ISO 9660 primary volume descriptor was not found.");file_.close();return false;}
 DirectoryEntry boot{};if(findRootEntry("SYSTEM.CNF;1",boot,nullptr)){
  std::vector<std::uint8_t> bytes;if(readRootFile("SYSTEM.CNF;1",bytes,nullptr)){
   std::string cnf(bytes.begin(),bytes.end());const auto pos=upper(cnf).find("BOOT2");
   if(pos!=std::string::npos){const auto eq=cnf.find('=',pos);if(eq!=std::string::npos){auto start=cnf.find_first_not_of(" \t",eq+1);if(start!=std::string::npos){auto stop=cnf.find_first_of("\r\n \t",start);info_.bootFile=cnf.substr(start,stop==std::string::npos?std::string::npos:stop-start);const auto slash=info_.bootFile.find_last_of("/\\");auto file=info_.bootFile.substr(slash==std::string::npos?0:slash+1);const auto version=file.find(';');info_.serial=file.substr(0,version);}}}
  }
 }
 return true;
}
bool Iso9660Image::readSector(std::uint32_t lba,void* destination,std::size_t bytes,std::string* error)const{
 if(!file_.is_open()){setError(error,"No ISO image is mounted.");return false;}
 if(bytes!=kSectorSize||destination==nullptr){setError(error,"Sector reads require a non-null 2048-byte buffer.");return false;}
 if(lba>=info_.volumeSectors||(static_cast<std::uint64_t>(lba)+1u)*kSectorSize>fileSize_){setError(error,"Requested ISO sector is outside the volume.");return false;}
 file_.clear();file_.seekg(static_cast<std::streamoff>(static_cast<std::uint64_t>(lba)*kSectorSize));
 file_.read(static_cast<char*>(destination),static_cast<std::streamsize>(bytes));if(!file_){setError(error,"Failed to read ISO sector.");return false;}return true;
}
bool Iso9660Image::readSectors(std::uint32_t firstLba,std::uint32_t count,void* destination,std::string* error)const{
 if(count==0)return true;
 if(destination==nullptr){setError(error,"Multi-sector destination buffer is null.");return false;}
 const auto end=static_cast<std::uint64_t>(firstLba)+count;
 if(end>info_.volumeSectors){setError(error,"Multi-sector read exceeds ISO volume.");return false;}
 auto* out=static_cast<std::uint8_t*>(destination);
 for(std::uint32_t i=0;i<count;++i)if(!readSector(firstLba+i,out+static_cast<std::size_t>(i)*kSectorSize,kSectorSize,error))return false;
 return true;
}
bool Iso9660Image::findRootEntry(const std::string& wanted,DirectoryEntry& result,std::string* error)const{
 if(!file_.is_open()){setError(error,"No ISO image is mounted.");return false;}
 std::vector<std::uint8_t> dir(rootSize_);file_.clear();file_.seekg(static_cast<std::streamoff>(static_cast<std::uint64_t>(rootLba_)*kSectorSize));file_.read(reinterpret_cast<char*>(dir.data()),static_cast<std::streamsize>(dir.size()));if(!file_){setError(error,"Could not read ISO root directory.");return false;}
 const auto target=upper(wanted);std::size_t pos=0;
 while(pos<dir.size()){
  const auto len=dir[pos];if(len==0){pos=((pos/kSectorSize)+1)*kSectorSize;continue;}
  if(len<34||pos+len>dir.size()){setError(error,"Malformed ISO directory record.");return false;}
  const auto nameLen=dir[pos+32];if(33u+nameLen>len){setError(error,"Malformed ISO directory name.");return false;}
  std::string name(reinterpret_cast<const char*>(dir.data()+pos+33),nameLen);
  if(name.size()==1&&static_cast<unsigned char>(name[0])<=1){pos+=len;continue;}
  if(upper(name)==target){result.lba=le32(dir.data()+pos+2);result.size=le32(dir.data()+pos+10);result.directory=(dir[pos+25]&2u)!=0;return true;}
  pos+=len;
 }
 setError(error,"File not found in ISO root directory: "+wanted);return false;
}
bool Iso9660Image::readRootFile(const std::string& name,std::vector<std::uint8_t>& data,std::string* error)const{
 DirectoryEntry entry;if(!findRootEntry(name,entry,error))return false;if(entry.directory){setError(error,"Requested ISO entry is a directory.");return false;}
 if(static_cast<std::uint64_t>(entry.lba)*kSectorSize+entry.size>fileSize_){setError(error,"ISO file extent is outside the image.");return false;}
 data.resize(entry.size);file_.clear();file_.seekg(static_cast<std::streamoff>(static_cast<std::uint64_t>(entry.lba)*kSectorSize));if(entry.size)file_.read(reinterpret_cast<char*>(data.data()),entry.size);if(!file_){data.clear();setError(error,"Could not read ISO file extent.");return false;}return true;
}
bool VirtualCdDvdDrive::mountIso(const std::filesystem::path& path,std::string* error){
 eject();if(!image_.open(path,error))return false;info_=image_.info();return true;
}
bool VirtualCdDvdDrive::mountUl(const std::filesystem::path& root,std::size_t gameIndex,std::string* error){
 eject();if(!ulLoader_.scan(root,error))return false;
 if(gameIndex>=ulLoader_.games().size()){setError(error,"UL game index is out of range.");return false;}
 ulGame_=ulLoader_.games()[gameIndex];ulMounted_=true;ulByteSize_=0;
 for(std::size_t part=0;part<ulGame_.parts;++part){
  std::error_code ec;const auto size=std::filesystem::file_size(ulLoader_.partPath(ulGame_,part),ec);
  if(ec||size==0||ulByteSize_>UINT64_MAX-size){eject();setError(error,"UL image contains a missing, empty or oversized part.");return false;}
  ulByteSize_+=size;
 }
 info_={};info_.volumeId=ulGame_.name;info_.serial=ulGame_.startup;info_.volumeSectors=static_cast<std::uint32_t>(std::min<std::uint64_t>((ulByteSize_+2047u)/2048u,UINT32_MAX));return true;
}
void VirtualCdDvdDrive::eject()noexcept{image_.close();ulMounted_=false;ulGame_={};ulByteSize_=0;info_={};}
bool VirtualCdDvdDrive::readSectors(std::uint32_t firstLba,std::uint32_t count,void* destination,std::string* error)const{
 if(count==0)return true;
 if(destination==nullptr){setError(error,"Multi-sector destination buffer is null.");return false;}
 const auto end=static_cast<std::uint64_t>(firstLba)+count;
 if(end>static_cast<std::uint64_t>(UINT32_MAX)+1u){setError(error,"Multi-sector LBA range overflow.");return false;}
 if(!ulMounted_)return image_.readSectors(firstLba,count,destination,error);
 auto* out=static_cast<std::uint8_t*>(destination);
 for(std::uint32_t i=0;i<count;++i)if(!readSector(firstLba+i,out+static_cast<std::size_t>(i)*Iso9660Image::kSectorSize,Iso9660Image::kSectorSize,error))return false;
 (void)end;return true;
}
bool VirtualCdDvdDrive::readSector(std::uint32_t lba,void* destination,std::size_t bytes,std::string* error)const{
 if(ulMounted_){
  if(bytes!=Iso9660Image::kSectorSize||destination==nullptr){setError(error,"Virtual CD/DVD reads require a non-null 2048-byte sector buffer.");return false;}
  if(lba>=sectorCount()){setError(error,"Requested UL sector is outside the mounted image.");return false;}
  const auto offset=static_cast<std::uint64_t>(lba)*Iso9660Image::kSectorSize;
  const auto available=static_cast<std::size_t>(std::min<std::uint64_t>(bytes,ulByteSize_-offset));
  if(!ulLoader_.read(ulGame_,offset,destination,available,error))return false;
  if(available<bytes)std::fill(static_cast<std::uint8_t*>(destination)+available,static_cast<std::uint8_t*>(destination)+bytes,0);
  return true;
 }
 return image_.readSector(lba,destination,bytes,error);
}
}
