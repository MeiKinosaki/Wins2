#include "frontend/user_paths.hpp"
#include <filesystem>
#include <iostream>
int main(int argc,char** argv){
 std::error_code ec;
 const auto executable=std::filesystem::absolute(argc>0?argv[0]:"Wins2.exe",ec);
 const auto root=ec?std::filesystem::current_path():executable.parent_path();
 const auto paths=wins2::UserPaths::under(root);
 std::string error;
 if(!paths.createAll(&error)){std::cerr<<"Wins2: "<<error<<"\\n";return 1;}
 std::cout<<"Wins2 PlayStation 2 Emulator - development foundation\\n";
 std::cout<<"Portable data directory: "<<paths.root.string()<<"\\n";
 std::cout<<"Status: not yet capable of booting games.\\n";
 return 0;
}
