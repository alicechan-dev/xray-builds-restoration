#include "archive_reader.h"

#include "path_safety.h"
#include "xrCore.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <stdexcept>

// lzhuf.h marks this helper as XRCORE_API for DLL consumers, but xrCore keeps
// the LZHUF helpers private. xr_unpack compiles LzHuf.cpp locally, matching the
// read-only xrArchiveList diagnostic.
extern void __stdcall _decompressLZ(u8** dest, unsigned* dest_sz, void* src, unsigned src_sz);

namespace xr_unpack
{
namespace
{
const std::uint32_t kCompressMark = 1u << 31u;
const std::uint32_t kDirectoryChunk = 1u;
const std::uint64_t kMaxDirectorySize = 256ull * 1024ull * 1024ull;

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

std::uint32_t read_u32(const std::vector<unsigned char>& data, std::size_t& pos, const char* context)
{
    if (pos + sizeof(std::uint32_t) > data.size())
        throw std::runtime_error(std::string("unexpected end of data while reading ") + context);

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

std::vector<unsigned char> load_file(const std::string& path, std::uint64_t& archive_size)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file)
        throw std::runtime_error("cannot open archive");

    file.seekg(0, std::ios::end);
    const std::streamoff length = file.tellg();
    if (length < 0)
        throw std::runtime_error("cannot size archive");

    archive_size = static_cast<std::uint64_t>(length);
    if (archive_size > static_cast<std::uint64_t>(static_cast<std::size_t>(-1)))
        throw std::runtime_error("archive is too large for this diagnostic build");

    std::vector<unsigned char> data(static_cast<std::size_t>(archive_size));
    file.seekg(0, std::ios::beg);
    if (!data.empty())
        file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));

    if (!file)
        throw std::runtime_error("cannot read archive");

    return data;
}

std::vector<unsigned char> extract_directory_chunk(const std::vector<unsigned char>& archive, ArchiveInfo& info)
{
    std::size_t pos = 0;
    while (pos + 8 <= archive.size()) {
        const std::size_t chunk_header = pos;
        const std::uint32_t type = read_u32(archive, pos, "chunk type");
        const std::uint32_t size = read_u32(archive, pos, "chunk size");
        if (static_cast<std::uint64_t>(pos) + size > archive.size())
            throw std::runtime_error("chunk extends past archive end");

        if ((type & ~kCompressMark) == kDirectoryChunk) {
            info.directory_found = true;
            info.directory_compressed = (type & kCompressMark) != 0;
            info.directory_chunk_type = type;
            info.directory_packed_size = size;
            info.directory_payload_offset = static_cast<std::uint64_t>(pos);

            const unsigned char* chunk = archive.data() + pos;
            if (info.directory_compressed) {
                u8* decompressed = 0;
                unsigned decompressed_size = 0;
                _decompressLZ(&decompressed, &decompressed_size, const_cast<unsigned char*>(chunk), size);
                if (!decompressed || !decompressed_size)
                    throw std::runtime_error("failed to decompress archive directory");

                if (decompressed_size > kMaxDirectorySize) {
                    xr_free(decompressed);
                    throw std::runtime_error("decompressed directory is too large for read-only inspection");
                }

                std::vector<unsigned char> result(decompressed, decompressed + decompressed_size);
                xr_free(decompressed);
                info.directory_unpacked_size = decompressed_size;
                return result;
            }

            info.directory_unpacked_size = size;
            return std::vector<unsigned char>(chunk, chunk + size);
        }

        pos += size;
        if (pos == chunk_header)
            throw std::runtime_error("archive chunk parser did not advance");
    }

    throw std::runtime_error("archive directory chunk not found");
}

std::vector<ArchiveEntry> parse_directory(const std::vector<unsigned char>& directory, ArchiveInfo& info)
{
    std::vector<ArchiveEntry> entries;
    std::map<std::string, std::size_t> seen;
    std::size_t pos = 0;

    while (pos < directory.size()) {
        ArchiveEntry entry;
        entry.name = read_string_z(directory, pos);
        entry.offset = read_u32(directory, pos, "entry offset");
        entry.size_real = read_u32(directory, pos, "entry real size");
        entry.size_compressed = read_u32(directory, pos, "entry compressed size");

        const PathValidationResult path = validate_archive_entry_path(entry.name);
        entry.path_safe = path.ok;
        entry.normalized_name = path.normalized;
        entry.path_warning = path.reason;

        if (!entry.path_safe)
            info.warnings.push_back("unsafe entry path: " + entry.name + " (" + entry.path_warning + ")");

        const std::string key = lowercase(entry.normalized_name.empty() ? entry.name : entry.normalized_name);
        if (seen.find(key) != seen.end())
            info.warnings.push_back("duplicate entry path: " + entry.name);
        else
            seen[key] = entries.size();

        const std::uint64_t end = static_cast<std::uint64_t>(entry.offset) + entry.size_compressed;
        if (end > info.archive_size)
            info.warnings.push_back("entry extends past archive end: " + entry.name);

        if (entry.size_compressed > entry.size_real && entry.size_real != 0)
            info.warnings.push_back("entry packed size is larger than unpacked size: " + entry.name);

        entries.push_back(entry);
    }

    info.entry_count = entries.size();
    return entries;
}
}

ArchiveInfo inspect_archive_path(const std::string& path)
{
    ArchiveInfo info;
    info.path = path;
    info.family = has_xp_extension(path) ? ArchiveFamily::XpArchive : ArchiveFamily::Unknown;
    info.archive_size = 0;
    info.parsed = false;
    info.directory_found = false;
    info.directory_compressed = false;
    info.directory_chunk_type = 0;
    info.directory_packed_size = 0;
    info.directory_unpacked_size = 0;
    info.directory_payload_offset = 0;
    info.entry_count = 0;
    return info;
}

ArchiveContents read_archive(const std::string& path)
{
    ArchiveContents contents;
    contents.info = inspect_archive_path(path);

    try {
        const std::vector<unsigned char> archive = load_file(path, contents.info.archive_size);
        const std::vector<unsigned char> directory = extract_directory_chunk(archive, contents.info);
        contents.entries = parse_directory(directory, contents.info);
        contents.info.parsed = true;
    }
    catch (const std::exception& error) {
        contents.errors.push_back(error.what());
    }

    return contents;
}

const char* archive_family_name(ArchiveFamily family)
{
    switch (family) {
    case ArchiveFamily::XpArchive:
        return "X-Ray .xp* archive";
    default:
        return "unknown archive family";
    }
}

std::string parser_not_implemented_message()
{
    return "archive parsing is not implemented yet; format research is required";
}

std::string extraction_not_implemented_message()
{
    return "archive parsing is read-only currently; extraction is not implemented yet";
}
}
