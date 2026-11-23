#include "cpu/iop/r3000a.hpp"
namespace wins2 {
namespace { constexpr std::size_t kBadVAddr=8,kStatus=12,kCause=13,kEpc=14; constexpr std::uint32_t kExcCodeShift=2u; }
R3000A::R3000A(IopMemory& memory)noexcept:memory_(memory){reset();}
void R3000A::reset(std::uint32_t pc)noexcept{gpr_.fill(0);cop0_.fill(0);hi_=lo_=0;pc_=pc;last_instruction_=branch_target_=branch_pc_=bad_vaddr_=0;branch_pending_=halted_=false;cop0_[15]=0x0000001Fu;}
std::uint32_t R3000A::reg(std::size_t i)const noexcept{return i<kRegisterCount?gpr_[i]:0;}
void R3000A::setReg(std::size_t i,std::uint32_t v)noexcept{if(i>0&&i<kRegisterCount)gpr_[i]=v;}
std::uint32_t R3000A::cop0(std::size_t i)const noexcept{return i<kCop0RegisterCount?cop0_[i]:0;}
void R3000A::setCop0(std::size_t i,std::uint32_t v)noexcept{
 if(i>=kCop0RegisterCount)return;
 if(i==kCause){cop0_[i]=(cop0_[i]&~0x00000300u)|(v&0x00000300u);return;}
 if(i==kBadVAddr||i==kEpc||i==15)return;
 cop0_[i]=v;
}
void R3000A::updateCausePending()noexcept{}
void R3000A::setInterruptLine(std::uint8_t line,bool asserted)noexcept{
 if(line>=8)return;const auto bit=1u<<(8u+line);
 if(asserted)cop0_[kCause]|=bit;else cop0_[kCause]&=~bit;
}
bool R3000A::interruptPending()const noexcept{return (cop0_[kStatus]&kStatusIEc)!=0u&&(cop0_[kStatus]&cop0_[kCause]&kStatusInterruptMask)!=0u;}
void R3000A::scheduleBranch(std::uint32_t target,std::uint32_t branchPc)noexcept{branch_pending_=true;branch_target_=target;branch_pc_=branchPc;}
void R3000A::raiseException(IopException exception,std::uint32_t faultPc,bool inDelaySlot,std::uint32_t badAddress)noexcept{
 auto& status=cop0_[kStatus];auto& cause=cop0_[kCause];
 cause=(cause&~(kCauseExcCodeMask|kCauseBD))|(static_cast<std::uint32_t>(exception)<<kExcCodeShift);
 if(inDelaySlot){cause|=kCauseBD;cop0_[kEpc]=branch_pc_;}else cop0_[kEpc]=faultPc;
 if(exception==IopException::AddressLoad||exception==IopException::AddressStore){bad_vaddr_=badAddress;cop0_[kBadVAddr]=badAddress;}
 status=(status&~0x3Fu)|((status<<2u)&0x3Fu);
 branch_pending_=false;
 pc_=(status&kStatusBEV)?kBootstrapExceptionVector:kGeneralExceptionVector;
 halted_=false;
}
bool R3000A::step()noexcept{
 if(halted_)return false;
 if(interruptPending()){raiseException(IopException::Interrupt,pc_,branch_pending_);return true;}
 std::uint32_t ins=0;
 if(!memory_.read32(pc_,ins)){raiseException(IopException::AddressLoad,pc_,false,pc_);return true;}
 last_instruction_=ins;const auto here=pc_;const bool apply=branch_pending_;const auto target=branch_target_;branch_pending_=false;pc_+=4u;
 const std::uint32_t op=ins>>26u,rs=(ins>>21u)&31u,rt=(ins>>16u)&31u,rd=(ins>>11u)&31u,sa=(ins>>6u)&31u,fn=ins&63u;
 const std::uint16_t imm=static_cast<std::uint16_t>(ins);const std::int32_t simm=static_cast<std::int16_t>(imm);
 const std::uint32_t a=gpr_[rs],b=gpr_[rt];
 switch(op){
 case 0x00:
  switch(fn){
  case 0x00:setReg(rd,b<<sa);break;case 0x02:setReg(rd,b>>sa);break;case 0x03:setReg(rd,static_cast<std::uint32_t>(static_cast<std::int32_t>(b)>>sa));break;
  case 0x08:scheduleBranch(a,here);break;case 0x09:setReg(rd?rd:31u,here+8u);scheduleBranch(a,here);break;
  case 0x0C:raiseException(IopException::Syscall,here,apply);return true;
  case 0x0D:raiseException(IopException::Breakpoint,here,apply);return true;
  case 0x10:setReg(rd,hi_);break;case 0x11:hi_=a;break;case 0x12:setReg(rd,lo_);break;case 0x13:lo_=a;break;
  case 0x20:case 0x21:setReg(rd,a+b);break;case 0x22:case 0x23:setReg(rd,a-b);break;
  case 0x24:setReg(rd,a&b);break;case 0x25:setReg(rd,a|b);break;case 0x26:setReg(rd,a^b);break;case 0x27:setReg(rd,~(a|b));break;
  case 0x2A:setReg(rd,static_cast<std::int32_t>(a)<static_cast<std::int32_t>(b));break;case 0x2B:setReg(rd,a<b);break;
  default:raiseException(IopException::ReservedInstruction,here,apply);return true;
  }break;
 case 0x02:scheduleBranch((pc_&0xF0000000u)|((ins&0x03FFFFFFu)<<2u),here);break;
 case 0x03:setReg(31,here+8u);scheduleBranch((pc_&0xF0000000u)|((ins&0x03FFFFFFu)<<2u),here);break;
 case 0x04:if(a==b)scheduleBranch(pc_+static_cast<std::uint32_t>(simm*4),here);break;
 case 0x05:if(a!=b)scheduleBranch(pc_+static_cast<std::uint32_t>(simm*4),here);break;
 case 0x08:case 0x09:setReg(rt,a+static_cast<std::uint32_t>(simm));break;
 case 0x0A:setReg(rt,static_cast<std::int32_t>(a)<simm);break;case 0x0B:setReg(rt,a<static_cast<std::uint32_t>(simm));break;
 case 0x0C:setReg(rt,a&imm);break;case 0x0D:setReg(rt,a|imm);break;case 0x0E:setReg(rt,a^imm);break;case 0x0F:setReg(rt,static_cast<std::uint32_t>(imm)<<16u);break;
 case 0x10:
  if(rs==0x00)setReg(rt,cop0_[rd]);
  else if(rs==0x04)setCop0(rd,b);
  else if(rs==0x10&&fn==0x10){cop0_[kStatus]=(cop0_[kStatus]&~0x0Fu)|((cop0_[kStatus]>>2u)&0x0Fu);}
  else {raiseException(IopException::ReservedInstruction,here,apply);return true;}
  break;
 case 0x20:case 0x24:{const auto address=a+static_cast<std::uint32_t>(simm);std::uint8_t v=0;if(!memory_.read8(address,v)){raiseException(IopException::AddressLoad,here,apply,address);return true;}setReg(rt,op==0x20?static_cast<std::uint32_t>(static_cast<std::int32_t>(static_cast<std::int8_t>(v))):v);break;}
 case 0x23:{const auto address=a+static_cast<std::uint32_t>(simm);std::uint32_t v=0;if(!memory_.read32(address,v)){raiseException(IopException::AddressLoad,here,apply,address);return true;}setReg(rt,v);break;}
 case 0x28:{const auto address=a+static_cast<std::uint32_t>(simm);if(!memory_.write8(address,static_cast<std::uint8_t>(b))){raiseException(IopException::AddressStore,here,apply,address);return true;}break;}
 case 0x2B:{const auto address=a+static_cast<std::uint32_t>(simm);if(!memory_.write32(address,b)){raiseException(IopException::AddressStore,here,apply,address);return true;}break;}
 default:raiseException(IopException::ReservedInstruction,here,apply);return true;
 }
 gpr_[0]=0;if(apply)pc_=target;return true;
}
R3000AState R3000A::captureState()const noexcept{R3000AState s;s.gpr=gpr_;s.gpr[0]=0;s.cop0=cop0_;s.hi=hi_;s.lo=lo_;s.pc=pc_;s.lastInstruction=last_instruction_;s.branchTarget=branch_target_;s.branchPc=branch_pc_;s.badVaddr=bad_vaddr_;s.branchPending=branch_pending_;s.halted=halted_;return s;}
void R3000A::restoreState(const R3000AState&s)noexcept{gpr_=s.gpr;gpr_[0]=0;cop0_=s.cop0;hi_=s.hi;lo_=s.lo;pc_=s.pc;last_instruction_=s.lastInstruction;branch_target_=s.branchTarget;branch_pc_=s.branchPc;bad_vaddr_=s.badVaddr;branch_pending_=s.branchPending;halted_=s.halted;}
}
