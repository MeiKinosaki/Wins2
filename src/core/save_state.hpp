#pragma once
#include "core/memory.hpp"
#include "cpu/ee/r5900.hpp"
#include <filesystem>
#include <string>
namespace wins2 {
class SaveState {
public:
 [[nodiscard]] static bool save(const std::filesystem::path& file,const Memory& memory,const R5900& cpu,std::string* error=nullptr) noexcept;
 [[nodiscard]] static bool load(const std::filesystem::path& file,Memory& memory,R5900& cpu,std::string* error=nullptr) noexcept;
};
}
