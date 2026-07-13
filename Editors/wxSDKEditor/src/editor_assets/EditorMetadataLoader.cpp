#include "editor_assets/EditorMetadataLoader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <sstream>
#include <system_error>

namespace
{
std::string Trim(std::string value)
{
    const auto notSpace = [](unsigned char character) {
        return !std::isspace(character);
    };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(),
        value.end());
    return value;
}

std::string Lower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
    return value;
}

bool EqualPathPart(const std::filesystem::path& left,
    const std::filesystem::path& right)
{
#ifdef _WIN32
    return Lower(left.string()) == Lower(right.string());
#else
    return left == right;
#endif
}

bool IsInside(const std::filesystem::path& root,
    const std::filesystem::path& candidate)
{
    auto rootIt = root.begin();
    auto candidateIt = candidate.begin();
    for (; rootIt != root.end(); ++rootIt, ++candidateIt)
    {
        if (candidateIt == candidate.end() ||
            !EqualPathPart(*rootIt, *candidateIt))
            return false;
    }
    return true;
}

void AddDiagnostic(EditorImportedMetadata& result,
    EditorMetadataDiagnosticSeverity severity, const std::string& file,
    std::size_t line, std::string message)
{
    result.diagnostics.push_back({severity, file, line, std::move(message)});
}

bool Fail(EditorImportedMetadata& result, const std::string& file,
    std::size_t line, std::string message, std::string* reason)
{
    AddDiagnostic(result, EditorMetadataDiagnosticSeverity::Error,
        file, line, message);
    if (reason)
    {
        *reason = file;
        if (line != 0)
            *reason += ":" + std::to_string(line);
        if (!reason->empty())
            *reason += ": ";
        *reason += message;
    }
    return false;
}

struct ParseContext
{
    const EditorMetadataLimits& limits;
    std::filesystem::path root;
    EditorImportedMetadata result;
    std::set<std::string> loaded;
    std::set<std::string> active;
};

bool ParseText(ParseContext& context, std::string_view text,
    const std::string& sourceName, const std::filesystem::path* sourcePath,
    std::size_t depth, std::string* reason);

bool LoadFile(ParseContext& context, const std::filesystem::path& path,
    std::size_t depth, std::string* reason)
{
    std::error_code error;
    const std::filesystem::path canonical = std::filesystem::canonical(path, error);
    const std::string display = error ? path.generic_string() :
        std::filesystem::relative(canonical, context.root, error).generic_string();
    if (error || !IsInside(context.root, canonical))
        return Fail(context.result, display, 0,
            "metadata file is outside the configured root", reason);
    if (depth > context.limits.maxIncludeDepth)
        return Fail(context.result, display, 0,
            "metadata include depth limit exceeded", reason);

    const std::string key = Lower(canonical.generic_string());
    if (context.active.count(key))
        return Fail(context.result, display, 0,
            "metadata include cycle detected", reason);
    if (context.loaded.count(key))
        return true;
    if (context.loaded.size() >= context.limits.maxFiles)
        return Fail(context.result, display, 0,
            "metadata file-count limit exceeded", reason);

    const std::uintmax_t size = std::filesystem::file_size(canonical, error);
    if (error || size > context.limits.maxBytesPerFile ||
        size > context.limits.maxTotalBytes - context.result.totalBytes)
        return Fail(context.result, display, 0,
            "metadata input-size limit exceeded", reason);

    std::ifstream input(canonical, std::ios::binary);
    if (!input)
        return Fail(context.result, display, 0,
            "could not open metadata file", reason);
    std::string text(static_cast<std::size_t>(size), '\0');
    input.read(text.data(), static_cast<std::streamsize>(text.size()));
    if (!input && !text.empty())
        return Fail(context.result, display, 0,
            "could not read metadata file", reason);

    context.loaded.insert(key);
    context.active.insert(key);
    context.result.files.push_back(display);
    context.result.totalBytes += static_cast<std::size_t>(size);
    const bool parsed = ParseText(context, text, display, &canonical,
        depth, reason);
    context.active.erase(key);
    return parsed;
}

bool ParseText(ParseContext& context, std::string_view text,
    const std::string& sourceName, const std::filesystem::path* sourcePath,
    std::size_t depth, std::string* reason)
{
    std::istringstream input{std::string(text)};
    std::string line;
    EditorMetadataSection* current = nullptr;
    std::size_t lineNumber = 0;
    while (std::getline(input, line))
    {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.size() > context.limits.maxLineBytes)
            return Fail(context.result, sourceName, lineNumber,
                "metadata line-length limit exceeded", reason);
        const std::size_t comment = line.find(';');
        if (comment != std::string::npos)
            line.erase(comment);
        line = Trim(std::move(line));
        if (line.empty())
            continue;

        if (line.rfind("#include", 0) == 0)
        {
            if (!sourcePath)
                return Fail(context.result, sourceName, lineNumber,
                    "includes require a configured metadata root", reason);
            const std::size_t firstQuote = line.find('"');
            const std::size_t lastQuote = line.find_last_of('"');
            if (firstQuote == std::string::npos || lastQuote == firstQuote)
                return Fail(context.result, sourceName, lineNumber,
                    "malformed metadata include", reason);
            const std::filesystem::path includePath =
                line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            if (includePath.empty() || includePath.is_absolute() ||
                includePath.has_root_name())
                return Fail(context.result, sourceName, lineNumber,
                    "absolute or empty metadata include is not allowed", reason);
            const std::filesystem::path candidate =
                sourcePath->parent_path() / includePath;
            ++context.result.includesFollowed;
            if (!LoadFile(context, candidate, depth + 1, reason))
                return false;
            continue;
        }

        if (line.front() == '[')
        {
            const std::size_t close = line.find(']');
            if (close == std::string::npos || close == 1)
                return Fail(context.result, sourceName, lineNumber,
                    "malformed metadata section header", reason);
            const std::string name = Lower(Trim(line.substr(1, close - 1)));
            const bool duplicate = std::any_of(context.result.sections.begin(),
                context.result.sections.end(), [&name](const auto& section) {
                    return section.name == name;
                });
            if (duplicate)
                return Fail(context.result, sourceName, lineNumber,
                    "duplicate metadata section: " + name, reason);
            const std::string trailing = Trim(line.substr(close + 1));
            if (!trailing.empty())
                AddDiagnostic(context.result,
                    EditorMetadataDiagnosticSeverity::Warning, sourceName,
                    lineNumber, "section suffix ignored by build-1935 grammar");
            context.result.sections.push_back(
                {name, sourceName, lineNumber, {}});
            current = &context.result.sections.back();
            continue;
        }

        if (!current)
        {
            AddDiagnostic(context.result,
                EditorMetadataDiagnosticSeverity::Warning, sourceName,
                lineNumber, "metadata record outside a section was ignored");
            continue;
        }
        const std::size_t equals = line.find('=');
        std::string key = Trim(line.substr(0, equals));
        std::string value = equals == std::string::npos ? std::string() :
            Trim(line.substr(equals + 1));
        if (key.empty())
        {
            AddDiagnostic(context.result,
                EditorMetadataDiagnosticSeverity::Warning, sourceName,
                lineNumber, "empty metadata key was ignored");
            continue;
        }
        if (current->FindFirst(key))
            AddDiagnostic(context.result,
                EditorMetadataDiagnosticSeverity::Warning, sourceName,
                lineNumber, "duplicate metadata key retained: " + key);
        current->entries.push_back(
            {std::move(key), std::move(value), sourceName, lineNumber});
    }
    return true;
}
}

bool EditorMetadataLoader::LoadMetadataRoot(const std::filesystem::path& root,
    const std::filesystem::path& entryFile, EditorImportedMetadata& result,
    std::string* reason) const
{
    std::error_code error;
    const std::filesystem::path canonicalRoot =
        std::filesystem::canonical(root, error);
    if (error || !std::filesystem::is_directory(canonicalRoot))
    {
        if (reason) *reason = "metadata root is not a readable directory";
        return false;
    }
    ParseContext context{limits_, canonicalRoot};
    context.result.rootPath = canonicalRoot.generic_string();
    const std::filesystem::path candidate = entryFile.is_absolute()
        ? entryFile : canonicalRoot / entryFile;
    if (!LoadFile(context, candidate, 0, reason))
        return false;
    error.clear();
    context.result.entryFile = std::filesystem::relative(
        std::filesystem::canonical(candidate, error), canonicalRoot, error)
        .generic_string();
    result = std::move(context.result);
    return true;
}

bool EditorMetadataLoader::ParseMetadataText(std::string_view text,
    std::string sourceName, EditorImportedMetadata& result,
    std::string* reason) const
{
    if (text.size() > limits_.maxBytesPerFile ||
        text.size() > limits_.maxTotalBytes)
    {
        if (reason) *reason = "metadata input-size limit exceeded";
        return false;
    }
    ParseContext context{limits_, {}};
    context.result.entryFile = sourceName;
    context.result.files.push_back(sourceName);
    context.result.totalBytes = text.size();
    if (!ParseText(context, text, sourceName, nullptr, 0, reason))
        return false;
    result = std::move(context.result);
    return true;
}
