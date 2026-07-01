#include "archive_reader.h"

#include <algorithm>
#include <cctype>

namespace xr_unpack
{
namespace
{
std::string lowercase(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

bool has_xp_extension(const std::string& path)
{
    const std::string value = lowercase(path);
    const std::size_t dot = value.find_last_of('.');
    if (dot == std::string::npos)
        return false;

    const std::string ext = value.substr(dot);
    if (ext.size() < 4)
        return false;

    if (ext[0] != '.' || ext[1] != 'x' || ext[2] != 'p')
        return false;

    for (std::size_t i = 3; i < ext.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(ext[i])))
            return false;
    }

    return true;
}
}

ArchiveInfo inspect_archive_path(const std::string& path)
{
    ArchiveInfo info;
    info.path = path;
    info.family = has_xp_extension(path) ? ArchiveFamily::XpArchive : ArchiveFamily::Unknown;
    return info;
}

const char* archive_family_name(ArchiveFamily family)
{
    switch (family) {
    case ArchiveFamily::XpArchive:
        return "X-Ray .xp* archive candidate";
    default:
        return "unknown archive family";
    }
}

std::string parser_not_implemented_message()
{
    return "archive parsing is not implemented yet; format research is required";
}
}
