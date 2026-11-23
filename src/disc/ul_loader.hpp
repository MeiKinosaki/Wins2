#pragma once
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>
namespace wins2 {
struct UlGame {
 std::string name;
 std::string startup;
 std::uint8_t parts{0};
 std::uint8_t media{0};
 std::uint32_t nameCrc{0};
};
class UlLoader {
public:
 static constexpr std::size_t kRecordSize=64;
 static constexpr std::size_t kChunkSize=1024ull*1024ull*1024ull;
 [[nodiscard]] bool scan(const std::filesystem::path& root,std::string* error=nullptr);
 [[nodiscard]] const std::vector<UlGame>& games()const noexcept{return games_;}
 [[nodiscard]] std::filesystem::path partPath(const UlGame& game,std::size_t part)const;
 [[nodiscard]] bool read(const UlGame& game,std::uint64_t offset,void* destination,std::size_t bytes,std::string* error=nullptr)const;
private:
 std::filesystem::path root_;
 std::vector<UlGame> games_;
};
}
