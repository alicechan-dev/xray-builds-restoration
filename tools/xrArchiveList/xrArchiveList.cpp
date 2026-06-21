// Read-only restoration diagnostic for X-Ray .xp* archives.
// Lists the same virtual entry names that LocatorAPI::ProcessArchive registers.

#include "xrCore.h"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// lzhuf.h marks this helper as XRCORE_API for DLL consumers, but xrCore keeps
// the LZHUF helpers private. This diagnostic tool compiles LzHuf.cpp locally.
extern void __stdcall _decompressLZ(u8** dest, unsigned* dest_sz, void* src, unsigned src_sz);

namespace
{
constexpr std::uint32_t kCompressMark = 1u << 31u;
constexpr std::uint32_t kDirectoryChunk = 1u;

std::uint32_t read_u32(const std::vector<unsigned char>& data, std::size_t& pos)
{
    if (pos + sizeof(std::uint32_t) > data.size())
        throw std::runtime_error("unexpected end of archive");

    std::uint32_t value = 0;
    value |= std::uint32_t(data[pos + 0]);
    value |= std::uint32_t(data[pos + 1]) << 8;
    value |= std::uint32_t(data[pos + 2]) << 16;
    value |= std::uint32_t(data[pos + 3]) << 24;
    pos += sizeof(std::uint32_t);
    return value;
}

std::string read_string_z(const std::vector<unsigned char>& data, std::size_t& pos)
{
    const std::size_t start = pos;
    while (pos < data.size() && data[pos])
        ++pos;

    if (pos >= data.size())
        throw std::runtime_error("unterminated archive entry name");

    std::string result(reinterpret_cast<const char*>(&data[start]), pos - start);
    ++pos;
    return result;
}

std::vector<unsigned char> load_file(const char* path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error(std::string("cannot open archive: ") + path);

    file.seekg(0, std::ios::end);
    const std::streamoff length = file.tellg();
    if (length < 0)
        throw std::runtime_error(std::string("cannot size archive: ") + path);

    std::vector<unsigned char> data(static_cast<std::size_t>(length));
    file.seekg(0, std::ios::beg);
    if (!data.empty())
        file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));

    if (!file)
        throw std::runtime_error(std::string("cannot read archive: ") + path);

    return data;
}

std::vector<unsigned char> find_directory_chunk(const std::vector<unsigned char>& archive)
{
    std::size_t pos = 0;
    while (pos + 8 <= archive.size()) {
        const std::uint32_t type = read_u32(archive, pos);
        const std::uint32_t size = read_u32(archive, pos);
        if (pos + size > archive.size())
            throw std::runtime_error("chunk extends past archive end");

        if ((type & ~kCompressMark) == kDirectoryChunk) {
            const unsigned char* chunk = archive.data() + pos;
            if (type & kCompressMark) {
                u8* decompressed = nullptr;
                unsigned decompressed_size = 0;
                _decompressLZ(&decompressed, &decompressed_size, const_cast<unsigned char*>(chunk), size);
                if (!decompressed || !decompressed_size)
                    throw std::runtime_error("failed to decompress archive directory");

                std::vector<unsigned char> result(decompressed, decompressed + decompressed_size);
                xr_free(decompressed);
                return result;
            }

            return std::vector<unsigned char>(chunk, chunk + size);
        }

        pos += size;
    }

    throw std::runtime_error("archive directory chunk not found");
}

std::string archive_base_name(const char* archive_path)
{
    std::string path(archive_path);
    const std::size_t slash = path.find_last_of("\\/");
    const std::size_t name_start = (slash == std::string::npos) ? 0 : slash + 1;
    const std::size_t dot = path.find_last_of('.');
    const std::size_t name_end = (dot == std::string::npos || dot < name_start) ? path.size() : dot;
    return path.substr(name_start, name_end - name_start) + "\\";
}
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "usage: xrArchiveList <archive.xp*>\n";
        return 2;
    }

    Core._initialize("xrArchiveList", nullptr, FALSE);

    try {
        const std::vector<unsigned char> archive = load_file(argv[1]);
        const std::vector<unsigned char> directory = find_directory_chunk(archive);
        const std::string base = archive_base_name(argv[1]);

        std::size_t pos = 0;
        while (pos < directory.size()) {
            const std::string name = read_string_z(directory, pos);
            read_u32(directory, pos);
            read_u32(directory, pos);
            read_u32(directory, pos);
            std::cout << base << name << '\n';
        }

        Core._destroy();
        return 0;
    }
    catch (const std::exception& error) {
        Core._destroy();
        std::cerr << "xrArchiveList: " << error.what() << '\n';
        return 1;
    }
}
