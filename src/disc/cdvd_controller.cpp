#include "disc/cdvd_controller.hpp"
#include <limits>
namespace wins2 {
bool CdvdController::refresh(std::string* error){
 buffer_.clear();lastLba_=0;
 if(!drive_.mounted()){status_=CdvdStatus::NoDisc;if(error)*error="No disc is mounted.";return false;}
 status_=CdvdStatus::Ready;return true;
}
bool CdvdController::read(std::uint32_t lba,std::uint32_t sectors,std::string* error){
 if(!drive_.mounted()){status_=CdvdStatus::NoDisc;if(error)*error="No disc is mounted.";return false;}
 if(sectors==0||sectors>32){status_=CdvdStatus::Error;if(error)*error="CD/DVD transfer must contain 1 to 32 sectors.";return false;}
 const auto bytes=static_cast<std::uint64_t>(sectors)*Iso9660Image::kSectorSize;
 if(bytes>std::numeric_limits<std::size_t>::max()){status_=CdvdStatus::Error;if(error)*error="CD/DVD transfer size overflow.";return false;}
 status_=CdvdStatus::Reading;
 try{buffer_.resize(static_cast<std::size_t>(bytes));}catch(...){buffer_.clear();status_=CdvdStatus::Error;if(error)*error="Could not allocate CD/DVD transfer buffer.";return false;}
 if(!drive_.readSectors(lba,sectors,buffer_.data(),error)){buffer_.clear();status_=CdvdStatus::Error;return false;}
 lastLba_=lba+sectors-1;status_=CdvdStatus::Ready;return true;
}
void CdvdController::reset()noexcept{buffer_.clear();lastLba_=0;status_=drive_.mounted()?CdvdStatus::Ready:CdvdStatus::NoDisc;}
}
