#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace xr_unpack
{
enum class ArchiveFamily
{
    Unknown,
    XpArchive
};

struct ArchiveInfo
{
    std::string path;
    ArchiveFamily family;
    std::uint64_t archive_size;
    bool parsed;
    bool directory_found;
    bool directory_compressed;
    std::uint32_t directory_chunk_type;
    std::uint32_t directory_packed_size;
    std::uint32_t directory_unpacked_size;
    std::uint64_t directory_payload_offset;
    std::size_t entry_count;
    std::vector<std::string> warnings;
};

struct ArchiveEntry
{
    std::string name;
    std::string normalized_name;
    std::uint32_t offset;
    std::uint32_t size_real;
    std::uint32_t size_compressed;
    bool path_safe;
    std::string path_warning;
};

struct ArchiveContents
{
    ArchiveInfo info;
    std::vector<ArchiveEntry> entries;
    std::vector<std::string> errors;
};

ArchiveInfo inspect_archive_path(const std::string& path);
ArchiveContents read_archive(const std::string& path);
const char* archive_family_name(ArchiveFamily family);
std::string parser_not_implemented_message();
std::string extraction_not_implemented_message();
}
