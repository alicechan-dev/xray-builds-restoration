#include "archive_reader.h"
#include "path_safety.h"

#include "xrCore.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace
{
enum ExitCode
{
    kOk = 0,
    kRuntimeError = 1,
    kUsageError = 2,
    kNotImplemented = 3
};

void print_help()
{
    std::cout
        << "xr_unpack - X-Ray archive research tool\n"
        << "\n"
        << "usage:\n"
        << "  xr_unpack help\n"
        << "  xr_unpack info <archive>\n"
        << "  xr_unpack list <archive> [--limit N]\n"
        << "  xr_unpack extract <archive> <out_dir> --dry-run [--limit N]\n"
        << "  xr_unpack verify <archive>\n"
        << "\n"
        << "info, list, and verify perform read-only inspection of proven .xp* archive\n"
        << "directory metadata. extract only supports dry-run planning in this phase.\n";
}

int require_arg_count(int argc, int expected, const char* usage)
{
    if (argc == expected)
        return kOk;

    std::cerr << "xr_unpack: invalid arguments\n";
    std::cerr << "usage: " << usage << "\n";
    return kUsageError;
}

void print_archive_errors(const xr_unpack::ArchiveContents& archive)
{
    for (std::vector<std::string>::const_iterator i = archive.errors.begin(); i != archive.errors.end(); ++i)
        std::cerr << "error: " << *i << "\n";
}

std::string lowercase(std::string value)
{
    for (std::string::iterator i = value.begin(); i != value.end(); ++i)
        *i = static_cast<char>(std::tolower(static_cast<unsigned char>(*i)));
    return value;
}

bool parse_limit_value(const char* value, std::size_t& limit)
{
    char* end = 0;
    const unsigned long parsed = std::strtoul(value, &end, 10);
    if (!value[0] || !end || *end)
        return false;

    limit = static_cast<std::size_t>(parsed);
    return true;
}

bool parse_limit_option(int argc, char** argv, int first_option, std::size_t& limit, bool& has_limit)
{
    has_limit = false;
    limit = 0;

    for (int i = first_option; i < argc; ++i) {
        const std::string option = argv[i];
        if (option == "--limit") {
            if (i + 1 >= argc) {
                std::cerr << "xr_unpack: --limit requires a numeric value\n";
                return false;
            }

            if (!parse_limit_value(argv[i + 1], limit)) {
                std::cerr << "xr_unpack: invalid --limit value '" << argv[i + 1] << "'\n";
                return false;
            }

            has_limit = true;
            ++i;
            continue;
        }

        std::cerr << "xr_unpack: unknown option '" << option << "'\n";
        return false;
    }

    return true;
}

int print_info(const char* archive_path)
{
    const xr_unpack::ArchiveContents archive = xr_unpack::read_archive(archive_path);
    const xr_unpack::ArchiveInfo& info = archive.info;

    std::cout << "archive: " << info.path << "\n";
    std::cout << "detected: " << xr_unpack::archive_family_name(info.family) << "\n";
    std::cout << "archive_size: " << info.archive_size << "\n";
    std::cout << "parser_status: " << (info.parsed ? "directory parsed" : "failed") << "\n";

    if (info.directory_found) {
        std::cout << "directory_chunk_type: 0x" << std::hex << info.directory_chunk_type << std::dec << "\n";
        std::cout << "directory_payload_offset: " << info.directory_payload_offset << "\n";
        std::cout << "directory_packed_size: " << info.directory_packed_size << "\n";
        std::cout << "directory_unpacked_size: " << info.directory_unpacked_size << "\n";
        std::cout << "directory_compressed: " << (info.directory_compressed ? "yes" : "no") << "\n";
    }

    std::cout << "file_count: " << info.entry_count << "\n";

    for (std::vector<std::string>::const_iterator i = info.warnings.begin(); i != info.warnings.end(); ++i)
        std::cout << "warning: " << *i << "\n";

    if (!archive.errors.empty()) {
        print_archive_errors(archive);
        return kRuntimeError;
    }

    return kOk;
}

int print_list(const char* archive_path, std::size_t limit, bool has_limit)
{
    const xr_unpack::ArchiveContents archive = xr_unpack::read_archive(archive_path);
    if (!archive.errors.empty()) {
        print_archive_errors(archive);
        return kRuntimeError;
    }

    std::cout << "path\toffset\tpacked_size\tunpacked_size\tflags\n";
    std::size_t printed = 0;
    for (std::vector<xr_unpack::ArchiveEntry>::const_iterator i = archive.entries.begin(); i != archive.entries.end(); ++i) {
        if (has_limit && printed >= limit)
            break;

        std::cout
            << i->name << "\t"
            << i->offset << "\t"
            << i->size_compressed << "\t"
            << i->size_real << "\t"
            << (i->size_real == i->size_compressed ? "stored" : "compressed");
        if (!i->path_safe)
            std::cout << ";unsafe-path";
        std::cout << "\n";
        ++printed;
    }

    if (has_limit && archive.entries.size() > printed)
        std::cout << "# output limited to " << printed << " of " << archive.entries.size() << " entries\n";

    return kOk;
}

int verify_archive(const char* archive_path)
{
    const xr_unpack::ArchiveContents archive = xr_unpack::read_archive(archive_path);
    if (!archive.errors.empty()) {
        print_archive_errors(archive);
        return kRuntimeError;
    }

    std::size_t unsafe_paths = 0;
    std::size_t out_of_bounds = 0;
    std::set<std::string> names;
    std::size_t duplicates = 0;

    for (std::vector<xr_unpack::ArchiveEntry>::const_iterator i = archive.entries.begin(); i != archive.entries.end(); ++i) {
        if (!i->path_safe)
            ++unsafe_paths;

        const std::uint64_t end = static_cast<std::uint64_t>(i->offset) + i->size_compressed;
        if (end > archive.info.archive_size)
            ++out_of_bounds;

        if (i->path_safe) {
            const std::pair<std::set<std::string>::iterator, bool> inserted = names.insert(lowercase(i->normalized_name));
            if (!inserted.second)
                ++duplicates;
        }
    }

    std::cout << "archive: " << archive.info.path << "\n";
    std::cout << "status: " << ((unsafe_paths || out_of_bounds) ? "failed" : "ok") << "\n";
    std::cout << "entries: " << archive.entries.size() << "\n";
    std::cout << "unsafe_paths: " << unsafe_paths << "\n";
    std::cout << "duplicate_names: " << duplicates << "\n";
    std::cout << "out_of_bounds_entries: " << out_of_bounds << "\n";

    for (std::vector<std::string>::const_iterator i = archive.info.warnings.begin(); i != archive.info.warnings.end(); ++i)
        std::cout << "warning: " << *i << "\n";

    return (unsafe_paths || out_of_bounds) ? kRuntimeError : kOk;
}

int report_extract_disabled(const char* archive)
{
    const xr_unpack::ArchiveInfo info = xr_unpack::inspect_archive_path(archive);
    std::cerr << "xr_unpack extract: extraction is not enabled yet; use --dry-run to inspect planned output\n";
    std::cerr << "archive: " << info.path << "\n";
    std::cerr << "detected: " << xr_unpack::archive_family_name(info.family) << "\n";
    return kNotImplemented;
}

struct DryRunOptions
{
    bool dry_run;
    bool has_limit;
    std::size_t limit;
};

bool parse_extract_options(int argc, char** argv, DryRunOptions& options)
{
    options.dry_run = false;
    options.has_limit = false;
    options.limit = 0;

    for (int i = 4; i < argc; ++i) {
        const std::string option = argv[i];
        if (option == "--dry-run") {
            options.dry_run = true;
            continue;
        }

        if (option == "--limit") {
            if (i + 1 >= argc) {
                std::cerr << "xr_unpack: --limit requires a numeric value\n";
                return false;
            }

            if (!parse_limit_value(argv[i + 1], options.limit)) {
                std::cerr << "xr_unpack: invalid --limit value '" << argv[i + 1] << "'\n";
                return false;
            }

            options.has_limit = true;
            ++i;
            continue;
        }

        std::cerr << "xr_unpack: unknown extract option '" << option << "'\n";
        return false;
    }

    return true;
}

int plan_extract_dry_run(const char* archive_path, const char* output_dir, const DryRunOptions& options)
{
    const xr_unpack::ArchiveContents archive = xr_unpack::read_archive(archive_path);
    if (!archive.errors.empty()) {
        print_archive_errors(archive);
        return kRuntimeError;
    }

    std::size_t safe_entries = 0;
    std::size_t unsafe_entries = 0;
    std::size_t duplicate_outputs = 0;
    std::size_t existing_outputs = 0;
    std::size_t out_of_bounds = 0;
    std::uint64_t would_write_bytes = 0;
    std::map<std::string, std::string> planned_outputs;
    std::vector<std::string> planned_paths;

    for (std::vector<xr_unpack::ArchiveEntry>::const_iterator i = archive.entries.begin(); i != archive.entries.end(); ++i) {
        const xr_unpack::PathValidationResult output = xr_unpack::compose_output_path(output_dir, i->name);
        if (!output.ok) {
            ++unsafe_entries;
            continue;
        }

        ++safe_entries;
        would_write_bytes += i->size_real;

        const std::string output_key = lowercase(output.normalized);
        if (planned_outputs.find(output_key) != planned_outputs.end())
            ++duplicate_outputs;
        else
            planned_outputs[output_key] = i->name;

        if (xr_unpack::should_refuse_existing_output(output.normalized))
            ++existing_outputs;

        const std::uint64_t end = static_cast<std::uint64_t>(i->offset) + i->size_compressed;
        if (end > archive.info.archive_size)
            ++out_of_bounds;

        planned_paths.push_back(output.normalized);
    }

    std::cout << "archive: " << archive.info.path << "\n";
    std::cout << "output_dir: " << output_dir << "\n";
    std::cout << "mode: dry-run\n";
    std::cout << "entries: " << archive.entries.size() << "\n";
    std::cout << "safe_entries: " << safe_entries << "\n";
    std::cout << "unsafe_entries: " << unsafe_entries << "\n";
    std::cout << "duplicates: " << duplicate_outputs << "\n";
    std::cout << "existing_outputs: " << existing_outputs << "\n";
    std::cout << "out_of_bounds_entries: " << out_of_bounds << "\n";
    std::cout << "would_write_bytes: " << would_write_bytes << "\n";
    std::cout << "status: " << ((unsafe_entries || duplicate_outputs || out_of_bounds) ? "failed" : "ok") << "\n";

    const std::size_t print_count = options.has_limit ? std::min(options.limit, planned_paths.size()) : planned_paths.size();
    std::cout << "planned_paths:\n";
    for (std::size_t i = 0; i < print_count; ++i)
        std::cout << planned_paths[i] << "\n";

    if (options.has_limit && planned_paths.size() > print_count)
        std::cout << "# output limited to " << print_count << " of " << planned_paths.size() << " planned paths\n";

    std::cout << "no files were written\n";

    return (unsafe_entries || duplicate_outputs || out_of_bounds) ? kRuntimeError : kOk;
}
}

int main(int argc, char** argv)
{
    Core._initialize("xr_unpack", 0, FALSE);

    if (argc < 2) {
        print_help();
        Core._destroy();
        return kUsageError;
    }

    const std::string command = argv[1];
    if (command == "help" || command == "--help" || command == "-h") {
        print_help();
        Core._destroy();
        return kOk;
    }

    int result = kUsageError;

    if (command == "info") {
        const int check = require_arg_count(argc, 3, "xr_unpack info <archive>");
        if (check != kOk)
            result = check;
        else
            result = print_info(argv[2]);
    }
    else if (command == "list") {
        if (argc < 3) {
            std::cerr << "xr_unpack: invalid arguments\n";
            std::cerr << "usage: xr_unpack list <archive> [--limit N]\n";
            result = kUsageError;
        }
        else
        {
            std::size_t limit = 0;
            bool has_limit = false;
            if (!parse_limit_option(argc, argv, 3, limit, has_limit))
                result = kUsageError;
            else
                result = print_list(argv[2], limit, has_limit);
        }
    }
    else if (command == "verify") {
        const int check = require_arg_count(argc, 3, "xr_unpack verify <archive>");
        if (check != kOk)
            result = check;
        else
            result = verify_archive(argv[2]);
    }
    else if (command == "extract") {
        if (argc < 4) {
            std::cerr << "xr_unpack: invalid arguments\n";
            std::cerr << "usage: xr_unpack extract <archive> <out_dir> --dry-run [--limit N]\n";
            result = kUsageError;
        }
        else if (std::string(argv[3]).empty()) {
            std::cerr << "xr_unpack extract: output directory is empty\n";
            result = kUsageError;
        }
        else {
            DryRunOptions options;
            if (!parse_extract_options(argc, argv, options))
                result = kUsageError;
            else if (!options.dry_run)
                result = report_extract_disabled(argv[2]);
            else
                result = plan_extract_dry_run(argv[2], argv[3], options);
        }
    }
    else {
        std::cerr << "xr_unpack: unknown command '" << command << "'\n";
        print_help();
        result = kUsageError;
    }

    Core._destroy();
    return result;
}
