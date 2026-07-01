#include "archive_reader.h"
#include "path_safety.h"

#include <iostream>
#include <string>

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
        << "xr_unpack - X-Ray archive research/unpacking skeleton\n"
        << "\n"
        << "usage:\n"
        << "  xr_unpack help\n"
        << "  xr_unpack info <archive>\n"
        << "  xr_unpack list <archive>\n"
        << "  xr_unpack extract <archive> <out_dir>\n"
        << "  xr_unpack verify <archive>\n"
        << "\n"
        << "This initial restoration tool does not parse archives yet.\n"
        << "It is a CLI and safety scaffold for lawful archive format research.\n";
}

int require_arg_count(int argc, int expected, const char* usage)
{
    if (argc == expected)
        return kOk;

    std::cerr << "xr_unpack: invalid arguments\n";
    std::cerr << "usage: " << usage << "\n";
    return kUsageError;
}

int report_parser_placeholder(const char* command, const char* archive)
{
    const xr_unpack::ArchiveInfo info = xr_unpack::inspect_archive_path(archive);

    std::cerr << "xr_unpack " << command << ": " << xr_unpack::parser_not_implemented_message() << "\n";
    std::cerr << "archive: " << info.path << "\n";
    std::cerr << "detected: " << xr_unpack::archive_family_name(info.family) << "\n";
    return kNotImplemented;
}
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        print_help();
        return kUsageError;
    }

    const std::string command = argv[1];
    if (command == "help" || command == "--help" || command == "-h") {
        print_help();
        return kOk;
    }

    if (command == "info") {
        const int check = require_arg_count(argc, 3, "xr_unpack info <archive>");
        if (check != kOk)
            return check;
        return report_parser_placeholder("info", argv[2]);
    }

    if (command == "list") {
        const int check = require_arg_count(argc, 3, "xr_unpack list <archive>");
        if (check != kOk)
            return check;
        return report_parser_placeholder("list", argv[2]);
    }

    if (command == "verify") {
        const int check = require_arg_count(argc, 3, "xr_unpack verify <archive>");
        if (check != kOk)
            return check;
        return report_parser_placeholder("verify", argv[2]);
    }

    if (command == "extract") {
        const int check = require_arg_count(argc, 4, "xr_unpack extract <archive> <out_dir>");
        if (check != kOk)
            return check;

        if (std::string(argv[3]).empty()) {
            std::cerr << "xr_unpack extract: output directory is empty\n";
            return kUsageError;
        }

        return report_parser_placeholder("extract", argv[2]);
    }

    std::cerr << "xr_unpack: unknown command '" << command << "'\n";
    print_help();
    return kUsageError;
}
