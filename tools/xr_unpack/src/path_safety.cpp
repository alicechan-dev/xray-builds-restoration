#include "path_safety.h"

#include <cctype>
#include <fstream>
#include <vector>

namespace xr_unpack
{
namespace
{
bool is_separator(char c)
{
    return c == '/' || c == '\\';
}

bool has_drive_prefix(const std::string& path)
{
    return path.size() >= 2 &&
        std::isalpha(static_cast<unsigned char>(path[0])) &&
        path[1] == ':';
}

bool is_absolute_path(const std::string& path)
{
    return !path.empty() && (is_separator(path[0]) || has_drive_prefix(path));
}

std::string trim_trailing_separators(std::string path)
{
    while (!path.empty() && is_separator(path[path.size() - 1]))
        path.erase(path.size() - 1);
    return path;
}

std::vector<std::string> split_components(const std::string& path)
{
    std::vector<std::string> result;
    std::string current;

    for (std::string::const_iterator i = path.begin(); i != path.end(); ++i) {
        if (is_separator(*i)) {
            result.push_back(current);
            current.clear();
            continue;
        }

        current.push_back(*i);
    }

    result.push_back(current);
    return result;
}

std::string join_components(const std::vector<std::string>& components)
{
    std::string result;
    for (std::vector<std::string>::const_iterator i = components.begin(); i != components.end(); ++i) {
        if (!result.empty())
            result += '/';
        result += *i;
    }
    return result;
}
}

PathValidationResult validate_archive_entry_path(const std::string& entry_path)
{
    PathValidationResult result;
    result.ok = false;

    if (entry_path.empty()) {
        result.reason = "archive entry path is empty";
        return result;
    }

    if (is_absolute_path(entry_path)) {
        result.reason = "archive entry path is absolute";
        return result;
    }

    std::vector<std::string> normalized;
    const std::vector<std::string> components = split_components(entry_path);
    for (std::vector<std::string>::const_iterator i = components.begin(); i != components.end(); ++i) {
        if (i->empty() || *i == ".")
            continue;

        if (*i == "..") {
            result.reason = "archive entry path contains parent-directory traversal";
            return result;
        }

        if (has_drive_prefix(*i)) {
            result.reason = "archive entry path contains a drive-qualified component";
            return result;
        }

        normalized.push_back(*i);
    }

    if (normalized.empty()) {
        result.reason = "archive entry path has no file component";
        return result;
    }

    result.ok = true;
    result.normalized = join_components(normalized);
    return result;
}

PathValidationResult compose_output_path(const std::string& output_dir, const std::string& entry_path)
{
    PathValidationResult entry = validate_archive_entry_path(entry_path);
    if (!entry.ok)
        return entry;

    PathValidationResult result;
    result.ok = false;

    if (output_dir.empty()) {
        result.reason = "output directory is empty";
        return result;
    }

    const std::string base = trim_trailing_separators(output_dir);
    if (base.empty()) {
        result.reason = "output directory resolves to an empty base";
        return result;
    }

    result.ok = true;
    result.normalized = base + "/" + entry.normalized;
    return result;
}

bool should_refuse_existing_output(const std::string& output_path)
{
    std::ifstream existing(output_path.c_str(), std::ios::binary);
    return existing.good();
}
}
