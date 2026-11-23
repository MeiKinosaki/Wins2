#pragma once
#include <filesystem>
#include <string>
namespace wins2 {
struct UserPaths {
 std::filesystem::path root,games,covers,saves,states,bios,screenshots,config,shaders,cache,logs;
 [[nodiscard]] static UserPaths under(const std::filesystem::path& rootDirectory);
 [[nodiscard]] bool createAll(std::string* error=nullptr) const noexcept;
 [[nodiscard]] std::filesystem::path gameSaveDirectory(const std::string& gameId) const;
 [[nodiscard]] std::filesystem::path gameStateDirectory(const std::string& gameId) const;
};
}
