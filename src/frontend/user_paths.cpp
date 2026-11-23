#include "frontend/user_paths.hpp"
#include <system_error>
namespace wins2 {
UserPaths UserPaths::under(const std::filesystem::path& base){
 UserPaths p;
 p.root=base;p.games=base/"games";p.covers=base/"covers";p.saves=base/"saves";
 p.states=base/"states";p.bios=base/"bios";p.screenshots=base/"screenshots";
 p.config=base/"config";p.shaders=base/"shaders";p.cache=base/"cache";p.logs=base/"logs";
 return p;
}
bool UserPaths::createAll(std::string* error) const noexcept {
 const std::filesystem::path* dirs[]={&root,&games,&covers,&saves,&states,&bios,&screenshots,&config,&shaders,&cache,&logs};
 std::error_code ec;
 for(const auto* dir:dirs){
  std::filesystem::create_directories(*dir,ec);
  if(ec){if(error)*error="Cannot create "+dir->string()+": "+ec.message();return false;}
 }
 return true;
}
std::filesystem::path UserPaths::gameSaveDirectory(const std::string& id) const{return saves/id;}
std::filesystem::path UserPaths::gameStateDirectory(const std::string& id) const{return states/id;}
}
