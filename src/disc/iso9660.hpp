#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include "disc/ul_loader.hpp"

namespace wins2 {
struct IsoDiscInfo {
 std::string volumeId;
 std::string bootFile;
 std::string serial;
 std::uint32_t volumeSectors{0};
};
class Iso9660Image {
public:
 static constexpr std::size_t kSectorSize=2048;
 [[nodiscard]] bool open(const std::filesystem::path& path,std::string* error=nullptr);
 void close() noexcept;
 [[nodiscard]] bool readSector(std::uint32_t lba,void* destination,std::size_t bytes=kSectorSize,std::string* error=nullptr) const;
 [[nodiscard]] bool readRootFile(const std::string& name,std::vector<std::uint8_t>& data,std::string* error=nullptr) const;
 [[nodiscard]] const IsoDiscInfo& info() const noexcept{return info_;}
 [[nodiscard]] bool isOpen() const noexcept{return file_.is_open();}
private:
 struct DirectoryEntry {std::uint32_t lba{0};std::uint32_t size{0};bool directory{false};};
 [[nodiscard]] bool findRootEntry(const std::string& name,DirectoryEntry& entry,std::string* error) const;
 std::filesystem::path path_;
 mutable std::ifstream file_;
 std::uint64_t fileSize_{0};
 std::uint32_t rootLba_{0},rootSize_{0};
 IsoDiscInfo info_;
};
class VirtualCdDvdDrive {
public:
 [[nodiscard]] bool mountIso(const std::filesystem::path& path,std::string* error=nullptr);
 [[nodiscard]] bool mountUl(const std::filesystem::path& root,std::size_t gameIndex,std::string* error=nullptr);
 void eject() noexcept;
 [[nodiscard]] bool readSector(std::uint32_t lba,void* destination,std::size_t bytes=Iso9660Image::kSectorSize,std::string* error=nullptr) const;
 [[nodiscard]] bool mounted() const noexcept{return image_.isOpen()||ulMounted_;}
 [[nodiscard]] const IsoDiscInfo& discInfo() const noexcept{return info_;}
private:
 Iso9660Image image_;
 UlLoader ulLoader_;
 UlGame ulGame_;
 IsoDiscInfo info_;
 bool ulMounted_{false};
};
}
