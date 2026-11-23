#include "cpu/ee/r5900.hpp"
#include <limits>
namespace wins2 {
R5900::R5900(Memory& memory) noexcept:memory_(memory){reset();}
void R5900::reset(std::uint32_t pc) noexcept{gpr_.fill(0);hi_=0;lo_=0;pc_=pc;last_instruction_=0;branch_pending_=false;branch_target_=0;halted_=false;}
std::uint64_t R5900::reg(std::size_t i) const noexcept{return i<kRegisterCount?gpr_[i]:0;}
void R5900::setReg(std::size_t i,std::uint64_t v) noexcept{if(i!=0&&i<kRegisterCount)gpr_[i]=v;}
std::uint64_t R5900::signExtend32(std::uint32_t v) noexcept{return static_cast<std::uint64_t>(static_cast<std::int64_t>(static_cast<std::int32_t>(v)));}
std::int64_t R5900::signExtend16(std::uint16_t v) noexcept{return static_cast<std::int16_t>(v);}
void R5900::scheduleBranch(std::uint32_t target) noexcept{branch_pending_=true;branch_target_=target;}
bool R5900::step() noexcept{
 if(halted_)return false;
 std::uint32_t ins=0;if(!memory_.read32(pc_,ins)){halted_=true;return false;}
 last_instruction_=ins;const auto here=pc_;const bool apply=branch_pending_;const auto target=branch_target_;branch_pending_=false;pc_+=4u;
 const auto op=ins>>26u,rs=(ins>>21u)&31u,rt=(ins>>16u)&31u,rd=(ins>>11u)&31u,sa=(ins>>6u)&31u,fn=ins&63u;
 const auto imm=static_cast<std::uint16_t>(ins);const auto simm=signExtend16(imm);const auto a=gpr_[rs],b=gpr_[rt];
 switch(op){
 case 0x00:
  switch(fn){
  case 0x00:setReg(rd,signExtend32(static_cast<std::uint32_t>(b)<<sa));break; // SLL
  case 0x02:setReg(rd,signExtend32(static_cast<std::uint32_t>(b)>>sa));break; // SRL
  case 0x03:setReg(rd,signExtend32(static_cast<std::uint32_t>(static_cast<std::int32_t>(b)>>sa)));break; // SRA
  case 0x08:scheduleBranch(static_cast<std::uint32_t>(a));break; // JR
  case 0x09:setReg(rd==0?31:rd,here+8u);scheduleBranch(static_cast<std::uint32_t>(a));break; // JALR
  case 0x10:setReg(rd,hi_);break; // MFHI
  case 0x11:hi_=a;break; // MTHI
  case 0x12:setReg(rd,lo_);break; // MFLO
  case 0x13:lo_=a;break; // MTLO
  case 0x18:{const auto product=static_cast<std::int64_t>(static_cast<std::int32_t>(a))*static_cast<std::int64_t>(static_cast<std::int32_t>(b));const auto bits=static_cast<std::uint64_t>(product);lo_=signExtend32(static_cast<std::uint32_t>(bits));hi_=signExtend32(static_cast<std::uint32_t>(bits>>32u));break;} // MULT
  case 0x19:{const auto product=static_cast<std::uint64_t>(static_cast<std::uint32_t>(a))*static_cast<std::uint64_t>(static_cast<std::uint32_t>(b));lo_=signExtend32(static_cast<std::uint32_t>(product));hi_=signExtend32(static_cast<std::uint32_t>(product>>32u));break;} // MULTU
  case 0x1A:{const auto x=static_cast<std::int32_t>(a),y=static_cast<std::int32_t>(b);if(y==0){hi_=signExtend32(static_cast<std::uint32_t>(x));lo_=x<0?1u:0xFFFFFFFFu;}else if(x==std::numeric_limits<std::int32_t>::min()&&y==-1){hi_=0;lo_=signExtend32(0x80000000u);}else{hi_=signExtend32(static_cast<std::uint32_t>(x%y));lo_=signExtend32(static_cast<std::uint32_t>(x/y));}break;} // DIV
  case 0x1B:{const auto x=static_cast<std::uint32_t>(a),y=static_cast<std::uint32_t>(b);if(y==0){hi_=signExtend32(x);lo_=0xFFFFFFFFu;}else{hi_=signExtend32(x%y);lo_=signExtend32(x/y);}break;} // DIVU
  case 0x21:setReg(rd,signExtend32(static_cast<std::uint32_t>(a)+static_cast<std::uint32_t>(b)));break; // ADDU
  case 0x23:setReg(rd,signExtend32(static_cast<std::uint32_t>(a)-static_cast<std::uint32_t>(b)));break; // SUBU
  case 0x24:setReg(rd,a&b);break;case 0x25:setReg(rd,a|b);break;
  case 0x26:setReg(rd,a^b);break;case 0x27:setReg(rd,~(a|b));break;
  case 0x2A:setReg(rd,static_cast<std::int64_t>(a)<static_cast<std::int64_t>(b));break;
  case 0x2B:setReg(rd,a<b);break;
  case 0x2D:setReg(rd,a+b);break; // DADDU
  case 0x2F:setReg(rd,a-b);break; // DSUBU
  case 0x38:setReg(rd,b<<sa);break; // DSLL
  case 0x3A:setReg(rd,b>>sa);break; // DSRL
  case 0x3B:setReg(rd,static_cast<std::uint64_t>(static_cast<std::int64_t>(b)>>sa));break; // DSRA
  case 0x3C:setReg(rd,b<<(sa+32u));break; // DSLL32
  case 0x3E:setReg(rd,b>>(sa+32u));break; // DSRL32
  case 0x3F:setReg(rd,static_cast<std::uint64_t>(static_cast<std::int64_t>(b)>>(sa+32u)));break; // DSRA32
  default:halted_=true;return false;
  }break;
 case 0x02:scheduleBranch((here&0xF0000000u)|((ins&0x03FFFFFFu)<<2u));break;
 case 0x03:setReg(31,here+8u);scheduleBranch((here&0xF0000000u)|((ins&0x03FFFFFFu)<<2u));break;
 case 0x04:if(a==b)scheduleBranch(pc_+static_cast<std::uint32_t>(simm*4));break;
 case 0x05:if(a!=b)scheduleBranch(pc_+static_cast<std::uint32_t>(simm*4));break;
 case 0x09:setReg(rt,signExtend32(static_cast<std::uint32_t>(a+simm)));break;
 case 0x0A:setReg(rt,static_cast<std::int64_t>(a)<simm);break;
 case 0x0B:setReg(rt,a<static_cast<std::uint64_t>(simm));break;
 case 0x0C:setReg(rt,a&imm);break;case 0x0D:setReg(rt,a|imm);break;
 case 0x0E:setReg(rt,a^imm);break;
 case 0x0F:setReg(rt,signExtend32(static_cast<std::uint32_t>(imm)<<16u));break;
 case 0x19:setReg(rt,a+static_cast<std::uint64_t>(simm));break; // DADDIU
 default:halted_=true;return false;
 }
 gpr_[0]=0;if(apply)pc_=target;return true;
}

R5900State R5900::captureState() const noexcept {
 R5900State s; s.gpr=gpr_; s.gpr[0]=0; s.hi=hi_; s.lo=lo_; s.pc=pc_;
 s.last_instruction=last_instruction_; s.branch_target=branch_target_;
 s.branch_pending=branch_pending_; s.halted=halted_; return s;
}
void R5900::restoreState(const R5900State& s) noexcept {
 gpr_=s.gpr; gpr_[0]=0; hi_=s.hi; lo_=s.lo; pc_=s.pc;
 last_instruction_=s.last_instruction; branch_target_=s.branch_target;
 branch_pending_=s.branch_pending; halted_=s.halted;
}
}
