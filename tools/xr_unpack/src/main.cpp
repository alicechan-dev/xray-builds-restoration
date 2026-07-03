#include "archive_reader.h"
#include "path_safety.h"

#include "xrCore.h"
#include "rt_compressor.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <windows.h>

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
        << "  xr_unpack list <archive> [--limit N] [--filter PATTERN]\n"
        << "  xr_unpack extract <archive> <out_dir> --dry-run [--limit N] [--filter PATTERN]\n"
        << "  xr_unpack extract <archive> <out_dir> --write\n"
        << "  xr_unpack verify <archive>\n"
        << "\n"
        << "info, list, and verify perform read-only inspection of proven .xp* archive\n"
        << "directory metadata. extract writes files only when --write is passed.\n"
        << "--filter supports simple '*' and '?' wildcards for list and dry-run output.\n";
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

std::string normalized_match_text(std::string value)
{
    for (std::string::iterator i = value.begin(); i != value.end(); ++i) {
        if (*i == '\\')
            *i = '/';
        else
            *i = static_cast<char>(std::tolower(static_cast<unsigned char>(*i)));
    }

    return value;
}

bool wildcard_match(const char* pattern, const char* value)
{
    const char* star = 0;
    const char* retry = 0;

    while (*value) {
        if (*pattern == '?' || *pattern == *value) {
            ++pattern;
            ++value;
            continue;
        }

        if (*pattern == '*') {
            star = pattern++;
            retry = value;
            continue;
        }

        if (star) {
            pattern = star + 1;
            value = ++retry;
            continue;
        }

        return false;
    }

    while (*pattern == '*')
        ++pattern;

    return !*pattern;
}

bool entry_matches_filter(const xr_unpack::ArchiveEntry& entry, bool has_filter, const std::string& filter)
{
    if (!has_filter)
        return true;

    const std::string name = normalized_match_text(entry.normalized_name.empty() ? entry.name : entry.normalized_name);
    return wildcard_match(filter.c_str(), name.c_str());
}

struct ListOptions
{
    bool has_limit;
    std::size_t limit;
    bool has_filter;
    std::string filter;
};

bool parse_list_options(int argc, char** argv, int first_option, ListOptions& options)
{
    options.has_limit = false;
    options.limit = 0;
    options.has_filter = false;
    options.filter.clear();

    for (int i = first_option; i < argc; ++i) {
        const std::string option = argv[i];
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

        if (option == "--filter") {
            if (i + 1 >= argc) {
                std::cerr << "xr_unpack: --filter requires a pattern\n";
                return false;
            }

            options.has_filter = true;
            options.filter = normalized_match_text(argv[i + 1]);
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

int print_list(const char* archive_path, const ListOptions& options)
{
    const xr_unpack::ArchiveContents archive = xr_unpack::read_archive(archive_path);
    if (!archive.errors.empty()) {
        print_archive_errors(archive);
        return kRuntimeError;
    }

    std::cout << "path\toffset\tpacked_size\tunpacked_size\tflags\n";
    std::size_t printed = 0;
    std::size_t matched = 0;
    for (std::vector<xr_unpack::ArchiveEntry>::const_iterator i = archive.entries.begin(); i != archive.entries.end(); ++i) {
        if (!entry_matches_filter(*i, options.has_filter, options.filter))
            continue;

        ++matched;

        if (options.has_limit && printed >= options.limit)
            continue;

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

    if (options.has_filter)
        std::cout << "# matched " << matched << " of " << archive.entries.size() << " entries\n";

    if (options.has_limit && matched > printed)
        std::cout << "# output limited to " << printed << " of " << matched << " matched entries\n";

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
    std::cerr << "xr_unpack extract: extraction is not enabled by default; use --dry-run to inspect planned output or --write to extract\n";
    std::cerr << "archive: " << info.path << "\n";
    std::cerr << "detected: " << xr_unpack::archive_family_name(info.family) << "\n";
    return kNotImplemented;
}

struct ExtractOptions
{
    bool dry_run;
    bool write;
    bool has_limit;
    std::size_t limit;
    bool has_filter;
    std::string filter;
};

struct PlannedEntry
{
    const xr_unpack::ArchiveEntry* entry;
    std::string output_path;
    bool directory;
};

struct ExtractPlan
{
    xr_unpack::ArchiveContents archive;
    std::vector<PlannedEntry> entries;
    std::size_t safe_entries;
    std::size_t unsafe_entries;
    std::size_t duplicate_outputs;
    std::size_t existing_outputs;
    std::size_t out_of_bounds;
    std::uint64_t would_write_bytes;
};

bool parse_extract_options(int argc, char** argv, ExtractOptions& options)
{
    options.dry_run = false;
    options.write = false;
    options.has_limit = false;
    options.limit = 0;
    options.has_filter = false;
    options.filter.clear();

    for (int i = 4; i < argc; ++i) {
        const std::string option = argv[i];
        if (option == "--dry-run") {
            options.dry_run = true;
            continue;
        }

        if (option == "--write") {
            options.write = true;
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

        if (option == "--filter") {
            if (i + 1 >= argc) {
                std::cerr << "xr_unpack: --filter requires a pattern\n";
                return false;
            }

            options.has_filter = true;
            options.filter = normalized_match_text(argv[i + 1]);
            ++i;
            continue;
        }

        std::cerr << "xr_unpack: unknown extract option '" << option << "'\n";
        return false;
    }

    if (options.dry_run && options.write) {
        std::cerr << "xr_unpack: choose either --dry-run or --write, not both\n";
        return false;
    }

    if (options.write && options.has_limit) {
        std::cerr << "xr_unpack: --limit is only supported with --dry-run\n";
        return false;
    }

    if (options.write && options.has_filter) {
        std::cerr << "xr_unpack: --filter is currently supported only with list and extract --dry-run\n";
        return false;
    }

    return true;
}

bool entry_is_directory_placeholder(const xr_unpack::ArchiveEntry& entry)
{
    if (entry.size_real || entry.size_compressed || entry.name.empty())
        return false;

    const char last = entry.name[entry.name.size() - 1];
    return last == '/' || last == '\\';
}

bool path_exists_as_directory(const std::string& path)
{
    struct _stat info;
    return _stat(path.c_str(), &info) == 0 && (info.st_mode & _S_IFDIR) != 0;
}

bool path_exists(const std::string& path)
{
    struct _stat info;
    return _stat(path.c_str(), &info) == 0;
}

std::vector<std::string> output_ancestors(std::string output_key)
{
    std::vector<std::string> result;
    for (;;) {
        const std::size_t separator = output_key.find_last_of("/\\");
        if (separator == std::string::npos)
            break;

        output_key.erase(separator);
        if (!output_key.empty())
            result.push_back(output_key);
    }

    return result;
}

ExtractPlan build_extract_plan(const char* archive_path, const char* output_dir, const ExtractOptions& options)
{
    ExtractPlan plan;
    plan.archive = xr_unpack::read_archive(archive_path);
    plan.safe_entries = 0;
    plan.unsafe_entries = 0;
    plan.duplicate_outputs = 0;
    plan.existing_outputs = 0;
    plan.out_of_bounds = 0;
    plan.would_write_bytes = 0;

    std::map<std::string, std::string> planned_outputs;
    std::set<std::string> planned_files;

    for (std::vector<xr_unpack::ArchiveEntry>::const_iterator i = plan.archive.entries.begin(); i != plan.archive.entries.end(); ++i) {
        if (!entry_matches_filter(*i, options.has_filter, options.filter))
            continue;

        const xr_unpack::PathValidationResult output = xr_unpack::compose_output_path(output_dir, i->name);
        if (!output.ok) {
            ++plan.unsafe_entries;
            continue;
        }

        ++plan.safe_entries;
        plan.would_write_bytes += i->size_real;

        const bool directory = entry_is_directory_placeholder(*i);
        const std::string output_key = lowercase(output.normalized);
        if (planned_outputs.find(output_key) != planned_outputs.end())
            ++plan.duplicate_outputs;
        else
            planned_outputs[output_key] = i->name;

        const std::vector<std::string> ancestors = output_ancestors(output_key);
        for (std::vector<std::string>::const_iterator ancestor = ancestors.begin(); ancestor != ancestors.end(); ++ancestor) {
            if (planned_files.find(*ancestor) != planned_files.end()) {
                ++plan.duplicate_outputs;
                break;
            }
        }

        if (!directory)
            planned_files.insert(output_key);

        if (directory) {
            if (path_exists(output.normalized) && !path_exists_as_directory(output.normalized))
                ++plan.existing_outputs;
        }
        else if (xr_unpack::should_refuse_existing_output(output.normalized))
            ++plan.existing_outputs;

        const std::uint64_t end = static_cast<std::uint64_t>(i->offset) + i->size_compressed;
        if (end > plan.archive.info.archive_size)
            ++plan.out_of_bounds;

        PlannedEntry planned;
        planned.entry = &(*i);
        planned.output_path = output.normalized;
        planned.directory = directory;
        plan.entries.push_back(planned);
    }

    return plan;
}

void print_extract_plan_summary(const ExtractPlan& plan, const char* output_dir, const char* mode)
{
    std::cout << "archive: " << plan.archive.info.path << "\n";
    std::cout << "output_dir: " << output_dir << "\n";
    std::cout << "mode: " << mode << "\n";
    std::cout << "entries: " << plan.entries.size() << "\n";
    if (plan.entries.size() != plan.archive.entries.size())
        std::cout << "archive_entries: " << plan.archive.entries.size() << "\n";
    std::cout << "safe_entries: " << plan.safe_entries << "\n";
    std::cout << "unsafe_entries: " << plan.unsafe_entries << "\n";
    std::cout << "duplicates: " << plan.duplicate_outputs << "\n";
    std::cout << "existing_outputs: " << plan.existing_outputs << "\n";
    std::cout << "out_of_bounds_entries: " << plan.out_of_bounds << "\n";
    std::cout << "would_write_bytes: " << plan.would_write_bytes << "\n";
    std::cout << "status: " << ((plan.unsafe_entries || plan.duplicate_outputs || plan.existing_outputs || plan.out_of_bounds) ? "failed" : "ok") << "\n";
}

bool extract_plan_is_safe_to_write(const ExtractPlan& plan)
{
    return plan.archive.errors.empty() &&
        !plan.unsafe_entries &&
        !plan.duplicate_outputs &&
        !plan.existing_outputs &&
        !plan.out_of_bounds;
}

int plan_extract_dry_run(const char* archive_path, const char* output_dir, const ExtractOptions& options)
{
    const ExtractPlan plan = build_extract_plan(archive_path, output_dir, options);
    if (!plan.archive.errors.empty()) {
        print_archive_errors(plan.archive);
        return kRuntimeError;
    }

    print_extract_plan_summary(plan, output_dir, "dry-run");

    const std::size_t print_count = options.has_limit ? std::min(options.limit, plan.entries.size()) : plan.entries.size();
    std::cout << "planned_paths:\n";
    for (std::size_t i = 0; i < print_count; ++i)
        std::cout << plan.entries[i].output_path << "\n";

    if (options.has_limit && plan.entries.size() > print_count)
        std::cout << "# output limited to " << print_count << " of " << plan.entries.size() << " planned paths\n";

    std::cout << "no files were written\n";

    return extract_plan_is_safe_to_write(plan) ? kOk : kRuntimeError;
}

bool create_directory_if_needed(const std::string& path)
{
    if (path.empty() || path_exists_as_directory(path))
        return true;

    if (CreateDirectoryA(path.c_str(), 0))
        return true;

    return GetLastError() == ERROR_ALREADY_EXISTS && path_exists_as_directory(path);
}

bool ensure_parent_directories(const std::string& file_path)
{
    const std::size_t last_separator = file_path.find_last_of("/\\");
    if (last_separator == std::string::npos)
        return true;

    const std::string directory = file_path.substr(0, last_separator);
    if (directory.empty())
        return true;

    std::string current;
    std::size_t pos = 0;
    if (directory.size() >= 2 && directory[1] == ':') {
        current = directory.substr(0, 2);
        pos = 2;
        if (pos < directory.size() && (directory[pos] == '/' || directory[pos] == '\\')) {
            current += directory[pos];
            ++pos;
        }
    }

    while (pos < directory.size()) {
        const std::size_t next = directory.find_first_of("/\\", pos);
        const std::string component = directory.substr(pos, next == std::string::npos ? std::string::npos : next - pos);
        if (!component.empty()) {
            if (!current.empty() && current[current.size() - 1] != '/' && current[current.size() - 1] != '\\')
                current += '/';
            current += component;
            if (!create_directory_if_needed(current))
                return false;
        }

        if (next == std::string::npos)
            break;
        pos = next + 1;
    }

    return true;
}

bool read_exact(std::ifstream& file, char* data, std::size_t size)
{
    if (!size)
        return true;

    file.read(data, static_cast<std::streamsize>(size));
    return file.gcount() == static_cast<std::streamsize>(size);
}

bool copy_stored_entry(std::ifstream& archive_file, std::ofstream& output_file, const xr_unpack::ArchiveEntry& entry, std::uint64_t& bytes_written)
{
    const std::size_t kChunkSize = 1024 * 1024;
    std::vector<char> buffer(kChunkSize);
    std::uint32_t remaining = entry.size_real;

    archive_file.seekg(entry.offset, std::ios::beg);
    if (!archive_file)
        return false;

    while (remaining) {
        const std::size_t request = std::min<std::size_t>(buffer.size(), remaining);
        if (!read_exact(archive_file, buffer.data(), request))
            return false;

        output_file.write(buffer.data(), static_cast<std::streamsize>(request));
        if (!output_file)
            return false;

        remaining -= static_cast<std::uint32_t>(request);
        bytes_written += request;
    }

    return true;
}

bool copy_compressed_entry(std::ifstream& archive_file, std::ofstream& output_file, const xr_unpack::ArchiveEntry& entry, std::uint64_t& bytes_written)
{
    const std::uint32_t kMaxBufferedEntrySize = 256u * 1024u * 1024u;
    if (entry.size_real > kMaxBufferedEntrySize || entry.size_compressed > kMaxBufferedEntrySize) {
        std::cerr << "xr_unpack extract: compressed entry is too large to buffer safely: " << entry.name << "\n";
        return false;
    }

    std::vector<unsigned char> compressed(entry.size_compressed);
    std::vector<unsigned char> decompressed(entry.size_real);

    archive_file.seekg(entry.offset, std::ios::beg);
    if (!archive_file)
        return false;

    if (!compressed.empty() && !read_exact(archive_file, reinterpret_cast<char*>(compressed.data()), compressed.size()))
        return false;

    const std::uint32_t decompressed_size = rtc_decompress(decompressed.empty() ? 0 : decompressed.data(), entry.size_real, compressed.empty() ? 0 : compressed.data(), entry.size_compressed);
    if (decompressed_size != entry.size_real) {
        std::cerr << "xr_unpack extract: decompressed size mismatch for entry: " << entry.name << "\n";
        return false;
    }

    if (!decompressed.empty()) {
        output_file.write(reinterpret_cast<const char*>(decompressed.data()), static_cast<std::streamsize>(decompressed.size()));
        if (!output_file)
            return false;
    }

    bytes_written += decompressed.size();
    return true;
}

int write_extract(const char* archive_path, const char* output_dir)
{
    ExtractOptions options;
    options.dry_run = false;
    options.write = true;
    options.has_limit = false;
    options.limit = 0;
    options.has_filter = false;
    options.filter.clear();

    const ExtractPlan plan = build_extract_plan(archive_path, output_dir, options);
    if (!plan.archive.errors.empty()) {
        print_archive_errors(plan.archive);
        return kRuntimeError;
    }

    print_extract_plan_summary(plan, output_dir, "write");

    if (!extract_plan_is_safe_to_write(plan)) {
        std::cerr << "xr_unpack extract: refusing to write because the extraction plan is not safe\n";
        return kRuntimeError;
    }

    std::ifstream archive_file(archive_path, std::ios::binary);
    if (!archive_file) {
        std::cerr << "xr_unpack extract: cannot open archive for reading: " << archive_path << "\n";
        return kRuntimeError;
    }

    std::size_t entries_written = 0;
    std::uint64_t bytes_written = 0;

    for (std::vector<PlannedEntry>::const_iterator i = plan.entries.begin(); i != plan.entries.end(); ++i) {
        if (i->directory) {
            if (!ensure_parent_directories(i->output_path)) {
                std::cerr << "xr_unpack extract: cannot create parent directory for: " << i->output_path << "\n";
                return kRuntimeError;
            }

            if (!create_directory_if_needed(i->output_path)) {
                std::cerr << "xr_unpack extract: cannot create output directory: " << i->output_path << "\n";
                return kRuntimeError;
            }

            ++entries_written;
            continue;
        }

        if (!ensure_parent_directories(i->output_path)) {
            std::cerr << "xr_unpack extract: cannot create output directory for: " << i->output_path << "\n";
            return kRuntimeError;
        }

        if (xr_unpack::should_refuse_existing_output(i->output_path)) {
            std::cerr << "xr_unpack extract: output already exists: " << i->output_path << "\n";
            return kRuntimeError;
        }

        std::ofstream output_file(i->output_path.c_str(), std::ios::binary);
        if (!output_file) {
            std::cerr << "xr_unpack extract: cannot create output file: " << i->output_path << "\n";
            return kRuntimeError;
        }

        const bool ok = (i->entry->size_real == i->entry->size_compressed)
            ? copy_stored_entry(archive_file, output_file, *i->entry, bytes_written)
            : copy_compressed_entry(archive_file, output_file, *i->entry, bytes_written);

        if (!ok) {
            std::cerr << "xr_unpack extract: failed while writing entry: " << i->entry->name << "\n";
            return kRuntimeError;
        }

        ++entries_written;
    }

    std::cout << "entries_written: " << entries_written << "\n";
    std::cout << "bytes_written: " << bytes_written << "\n";
    std::cout << "status: ok\n";
    return kOk;
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
            std::cerr << "usage: xr_unpack list <archive> [--limit N] [--filter PATTERN]\n";
            result = kUsageError;
        }
        else
        {
            ListOptions options;
            if (!parse_list_options(argc, argv, 3, options))
                result = kUsageError;
            else
                result = print_list(argv[2], options);
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
            std::cerr << "usage: xr_unpack extract <archive> <out_dir> (--dry-run [--limit N] [--filter PATTERN] | --write)\n";
            result = kUsageError;
        }
        else if (std::string(argv[3]).empty()) {
            std::cerr << "xr_unpack extract: output directory is empty\n";
            result = kUsageError;
        }
        else {
            ExtractOptions options;
            if (!parse_extract_options(argc, argv, options))
                result = kUsageError;
            else if (!options.dry_run)
                result = options.write ? write_extract(argv[2], argv[3]) : report_extract_disabled(argv[2]);
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
