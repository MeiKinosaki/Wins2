#include "core/save_state.hpp"
#include <array>
#include <cstdint>
#include <fstream>
#include <limits>
#include <vector>
namespace wins2 {
namespace {
constexpr std::array<char,8> kMagic={'W','I','N','S','2','S','T','A'};
constexpr std::uint32_t kVersion=1;
void put32(std::ostream& out,std::uint32_t v){for(unsigned i=0;i<4;++i)out.put(static_cast<char>((v>>(i*8u))&0xffu));}
void put64(std::ostream& out,std::uint64_t v){for(unsigned i=0;i<8;++i)out.put(static_cast<char>((v>>(i*8u))&0xffu));}
bool get32(std::istream& in,std::uint32_t& v){v=0;for(unsigned i=0;i<4;++i){int c=in.get();if(c==EOF)return false;v|=static_cast<std::uint32_t>(static_cast<unsigned char>(c))<<(i*8u);}return true;}
bool get64(std::istream& in,std::uint64_t& v){v=0;for(unsigned i=0;i<8;++i){int c=in.get();if(c==EOF)return false;v|=static_cast<std::uint64_t>(static_cast<unsigned char>(c))<<(i*8u);}return true;}
std::uint64_t hashByte(std::uint64_t h,std::uint8_t b){return (h^b)*1099511628211ull;}
std::uint64_t hashState(const R5900State& s,const std::vector<std::uint8_t>& ram){
 std::uint64_t h=1469598103934665603ull;
 auto add=[&](std::uint64_t v,unsigned n){for(unsigned i=0;i<n;++i)h=hashByte(h,static_cast<std::uint8_t>(v>>(i*8u)));};
 for(auto v:s.gpr)add(v,8);add(s.hi,8);add(s.lo,8);add(s.pc,4);add(s.last_instruction,4);add(s.branch_target,4);
 add(s.branch_pending?1:0,1);add(s.halted?1:0,1);
 for(auto b:ram)h=hashByte(h,b);
 return h;
}
void setError(std::string* e,const char* s){if(e)*e=s;}
}
bool SaveState::save(const std::filesystem::path& file,const Memory& memory,const R5900& cpu,std::string* error) noexcept{
 try{
  const auto state=cpu.captureState();const auto& ram=memory.ramData();
  std::ofstream out(file,std::ios::binary|std::ios::trunc);
  if(!out){setError(error,"Cannot open save-state file for writing.");return false;}
  out.write(kMagic.data(),static_cast<std::streamsize>(kMagic.size()));put32(out,kVersion);put32(out,static_cast<std::uint32_t>(ram.size()));
  for(auto v:state.gpr)put64(out,v);put64(out,state.hi);put64(out,state.lo);
  put32(out,state.pc);put32(out,state.last_instruction);put32(out,state.branch_target);
  out.put(state.branch_pending?1:0);out.put(state.halted?1:0);
  out.write(reinterpret_cast<const char*>(ram.data()),static_cast<std::streamsize>(ram.size()));
  put64(out,hashState(state,ram));out.flush();
  if(!out){setError(error,"Failed while writing save-state data.");return false;}
  return true;
 }catch(...){setError(error,"Unexpected error while saving state.");return false;}
}
bool SaveState::load(const std::filesystem::path& file,Memory& memory,R5900& cpu,std::string* error) noexcept{
 try{
  std::ifstream in(file,std::ios::binary);if(!in){setError(error,"Cannot open save-state file.");return false;}
  std::array<char,8> magic{};in.read(magic.data(),static_cast<std::streamsize>(magic.size()));
  if(!in||magic!=kMagic){setError(error,"Invalid save-state signature.");return false;}
  std::uint32_t version=0,size=0;if(!get32(in,version)||!get32(in,size)||version!=kVersion||size!=memory.ramSize()){setError(error,"Unsupported save-state version or RAM size.");return false;}
  R5900State state;
  for(auto& v:state.gpr)if(!get64(in,v)){setError(error,"Truncated CPU state.");return false;}
  if(!get64(in,state.hi)||!get64(in,state.lo)||!get32(in,state.pc)||!get32(in,state.last_instruction)||!get32(in,state.branch_target)){setError(error,"Truncated CPU state.");return false;}
  int bp=in.get(),halt=in.get();if(bp==EOF||halt==EOF||bp>1||halt>1){setError(error,"Invalid CPU flags.");return false;}
  state.branch_pending=bp!=0;state.halted=halt!=0;
  std::vector<std::uint8_t> ram(size);in.read(reinterpret_cast<char*>(ram.data()),static_cast<std::streamsize>(ram.size()));
  std::uint64_t expected=0;if(!in||!get64(in,expected)){setError(error,"Truncated RAM or checksum.");return false;}
  if(in.peek()!=EOF||hashState(state,ram)!=expected){setError(error,"Save-state checksum mismatch or trailing data.");return false;}
  if(!memory.restoreRam(ram)){setError(error,"Could not restore EE RAM.");return false;}
  cpu.restoreState(state);return true;
 }catch(...){setError(error,"Unexpected error while loading state.");return false;}
}
}
