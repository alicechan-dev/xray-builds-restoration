#pragma once

#include <string>

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
};

ArchiveInfo inspect_archive_path(const std::string& path);
const char* archive_family_name(ArchiveFamily family);
std::string parser_not_implemented_message();
}
