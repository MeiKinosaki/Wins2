#include "cpu/iop/r3000a.hpp"
namespace wins2 {
R3000A::R3000A(IopMemory& memory)noexcept:memory_(memory){reset();}
void R3000A::reset(std::uint32_t pc)noexcept{gpr_.fill(0);hi_=lo_=0;pc_=pc;last_instruction_=branch_target_=0;branch_pending_=halted_=false;}
std::uint32_t R3000A::reg(std::size_t i)const noexcept{return i<kRegisterCount?gpr_[i]:0;}
void R3000A::setReg(std::size_t i,std::uint32_t v)noexcept{if(i>0&&i<kRegisterCount)gpr_[i]=v;}
void R3000A::scheduleBranch(std::uint32_t target)noexcept{branch_pending_=true;branch_target_=target;}
bool R3000A::step()noexcept{
 if(halted_)return false;
 std::uint32_t ins=0;if(!memory_.read32(pc_,ins)){halted_=true;return false;}
 last_instruction_=ins;const auto here=pc_;const bool apply=branch_pending_;const auto target=branch_target_;branch_pending_=false;pc_+=4u;
 const std::uint32_t op=ins>>26u,rs=(ins>>21u)&31u,rt=(ins>>16u)&31u,rd=(ins>>11u)&31u,sa=(ins>>6u)&31u,fn=ins&63u;
 const std::uint16_t imm=static_cast<std::uint16_t>(ins);
 const std::int32_t simm=static_cast<std::int16_t>(imm);
 const std::uint32_t a=gpr_[rs],b=gpr_[rt];
 switch(op){
 case 0x00:
  switch(fn){
  case 0x00:setReg(rd,b<<sa);break;
  case 0x02:setReg(rd,b>>sa);break;
  case 0x03:setReg(rd,static_cast<std::uint32_t>(static_cast<std::int32_t>(b)>>sa));break;
  case 0x08:scheduleBranch(a);break;
  case 0x09:setReg(rd?rd:31u,here+8u);scheduleBranch(a);break;
  case 0x10:setReg(rd,hi_);break;case 0x11:hi_=a;break;
  case 0x12:setReg(rd,lo_);break;case 0x13:lo_=a;break;
  case 0x20:case 0x21:setReg(rd,a+b);break;
  case 0x22:case 0x23:setReg(rd,a-b);break;
  case 0x24:setReg(rd,a&b);break;case 0x25:setReg(rd,a|b);break;
  case 0x26:setReg(rd,a^b);break;case 0x27:setReg(rd,~(a|b));break;
  case 0x2A:setReg(rd,static_cast<std::int32_t>(a)<static_cast<std::int32_t>(b));break;
  case 0x2B:setReg(rd,a<b);break;
  default:halted_=true;return false;
  }break;
 case 0x02:scheduleBranch((pc_&0xF0000000u)|((ins&0x03FFFFFFu)<<2u));break;
 case 0x03:setReg(31,here+8u);scheduleBranch((pc_&0xF0000000u)|((ins&0x03FFFFFFu)<<2u));break;
 case 0x04:if(a==b)scheduleBranch(pc_+static_cast<std::uint32_t>(simm*4));break;
 case 0x05:if(a!=b)scheduleBranch(pc_+static_cast<std::uint32_t>(simm*4));break;
 case 0x08:case 0x09:setReg(rt,a+static_cast<std::uint32_t>(simm));break;
 case 0x0A:setReg(rt,static_cast<std::int32_t>(a)<simm);break;
 case 0x0B:setReg(rt,a<static_cast<std::uint32_t>(simm));break;
 case 0x0C:setReg(rt,a&imm);break;case 0x0D:setReg(rt,a|imm);break;
 case 0x0E:setReg(rt,a^imm);break;case 0x0F:setReg(rt,static_cast<std::uint32_t>(imm)<<16u);break;
 case 0x20:case 0x24:{std::uint8_t v=0;if(!memory_.read8(a+static_cast<std::uint32_t>(simm),v)){halted_=true;return false;}setReg(rt,op==0x20?static_cast<std::uint32_t>(static_cast<std::int32_t>(static_cast<std::int8_t>(v))):v);break;}
 case 0x23:{std::uint32_t v=0;if(!memory_.read32(a+static_cast<std::uint32_t>(simm),v)){halted_=true;return false;}setReg(rt,v);break;}
 case 0x28:if(!memory_.write8(a+static_cast<std::uint32_t>(simm),static_cast<std::uint8_t>(b))){halted_=true;return false;}break;
 case 0x2B:if(!memory_.write32(a+static_cast<std::uint32_t>(simm),b)){halted_=true;return false;}break;
 default:halted_=true;return false;
 }
 gpr_[0]=0;if(apply)pc_=target;return true;
}
R3000AState R3000A::captureState()const noexcept{R3000AState s;s.gpr=gpr_;s.gpr[0]=0;s.hi=hi_;s.lo=lo_;s.pc=pc_;s.lastInstruction=last_instruction_;s.branchTarget=branch_target_;s.branchPending=branch_pending_;s.halted=halted_;return s;}
void R3000A::restoreState(const R3000AState&s)noexcept{gpr_=s.gpr;gpr_[0]=0;hi_=s.hi;lo_=s.lo;pc_=s.pc;last_instruction_=s.lastInstruction;branch_target_=s.branchTarget;branch_pending_=s.branchPending;halted_=s.halted;}
}
