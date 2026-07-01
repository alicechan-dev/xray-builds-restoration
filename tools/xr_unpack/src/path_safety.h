#pragma once

#include <string>

namespace xr_unpack
{
struct PathValidationResult
{
    bool ok;
    std::string normalized;
    std::string reason;
};

PathValidationResult validate_archive_entry_path(const std::string& entry_path);
PathValidationResult compose_output_path(const std::string& output_dir, const std::string& entry_path);
bool should_refuse_existing_output(const std::string& output_path);
}
