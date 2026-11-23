#include "core/save_state.hpp"
#include "frontend/user_paths.hpp"
#include <filesystem>
#include <iostream>
int main(){
 const auto root=std::filesystem::temp_directory_path()/"wins2_storage_test";
 std::error_code ec;std::filesystem::remove_all(root,ec);
 auto paths=wins2::UserPaths::under(root);
 if(!paths.createAll())return 1;
 if(!std::filesystem::exists(paths.games)||!std::filesystem::exists(paths.covers)||!std::filesystem::exists(paths.saves)||!std::filesystem::exists(paths.states)||!std::filesystem::exists(paths.bios)||!std::filesystem::exists(paths.config))return 2;
 wins2::Memory mem;wins2::R5900 cpu(mem);
 if(!mem.write32(0,0x12345678u))return 3;
 cpu.setReg(3,0x1122334455667788ull);cpu.setPc(0x100);
 const auto stateFile=paths.gameStateDirectory("test-game")/"slot1.w2s";
 std::filesystem::create_directories(stateFile.parent_path());
 std::string error;
 if(!wins2::SaveState::save(stateFile,mem,cpu,&error)){std::cerr<<error<<"\\n";return 4;}
 if(!mem.write32(0,0)||!cpu.setReg(3,0),false){} // state is intentionally changed below
 cpu.setReg(3,0);cpu.setPc(0);
 if(!wins2::SaveState::load(stateFile,mem,cpu,&error)){std::cerr<<error<<"\\n";return 5;}
 std::uint32_t value=0;if(!mem.read32(0,value)||value!=0x12345678u||cpu.reg(3)!=0x1122334455667788ull||cpu.pc()!=0x100)return 6;
 std::filesystem::remove_all(root,ec);
 std::cout<<"Wins2 storage and save-state tests passed.\\n";return 0;
}
