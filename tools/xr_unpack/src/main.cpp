#include "archive_reader.h"
#include "path_safety.h"

#include "xrCore.h"

#include <cstdint>
#include <iomanip>
#include <iostream>
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
        << "  xr_unpack list <archive>\n"
        << "  xr_unpack extract <archive> <out_dir>\n"
        << "  xr_unpack verify <archive>\n"
        << "\n"
        << "info, list, and verify perform read-only inspection of proven .xp* archive\n"
        << "directory metadata. extract is intentionally disabled in this phase.\n";
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

int print_list(const char* archive_path)
{
    const xr_unpack::ArchiveContents archive = xr_unpack::read_archive(archive_path);
    if (!archive.errors.empty()) {
        print_archive_errors(archive);
        return kRuntimeError;
    }

    std::cout << "path\toffset\tpacked_size\tunpacked_size\tflags\n";
    for (std::vector<xr_unpack::ArchiveEntry>::const_iterator i = archive.entries.begin(); i != archive.entries.end(); ++i) {
        std::cout
            << i->name << "\t"
            << i->offset << "\t"
            << i->size_compressed << "\t"
            << i->size_real << "\t"
            << (i->size_real == i->size_compressed ? "stored" : "compressed");
        if (!i->path_safe)
            std::cout << ";unsafe-path";
        std::cout << "\n";
    }

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

        const std::pair<std::set<std::string>::iterator, bool> inserted = names.insert(i->normalized_name);
        if (!inserted.second)
            ++duplicates;
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
    std::cerr << "xr_unpack extract: " << xr_unpack::extraction_not_implemented_message() << "\n";
    std::cerr << "archive: " << info.path << "\n";
    std::cerr << "detected: " << xr_unpack::archive_family_name(info.family) << "\n";
    return kNotImplemented;
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
        const int check = require_arg_count(argc, 3, "xr_unpack list <archive>");
        if (check != kOk)
            result = check;
        else
            result = print_list(argv[2]);
    }
    else if (command == "verify") {
        const int check = require_arg_count(argc, 3, "xr_unpack verify <archive>");
        if (check != kOk)
            result = check;
        else
            result = verify_archive(argv[2]);
    }
    else if (command == "extract") {
        const int check = require_arg_count(argc, 4, "xr_unpack extract <archive> <out_dir>");
        if (check != kOk)
            result = check;
        else if (std::string(argv[3]).empty()) {
            std::cerr << "xr_unpack extract: output directory is empty\n";
            result = kUsageError;
        }
        else
            result = report_extract_disabled(argv[2]);
    }
    else {
        std::cerr << "xr_unpack: unknown command '" << command << "'\n";
        print_help();
        result = kUsageError;
    }

    Core._destroy();
    return result;
}
