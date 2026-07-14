#include "editor_assets/EditorObjectLibrary.h"

#include <algorithm>
#include <cctype>

namespace
{
bool EndsWithObject(std::string_view value)
{
    constexpr std::string_view suffix = ".object";
    if (value.size() < suffix.size()) return false;
    const auto tail = value.substr(value.size() - suffix.size());
    return std::equal(tail.begin(), tail.end(), suffix.begin(),
        [](unsigned char a, unsigned char b) {
            return std::tolower(a) == std::tolower(b);
        });
}
}

const char* ToString(EditorObjectKind value)
{
    switch (value) {
    case EditorObjectKind::Static: return "Static";
    case EditorObjectKind::Skeletal: return "Skeletal";
    default: return "Unknown";
    }
}

const char* ToString(EditorObjectParseStatus value)
{
    switch (value) {
    case EditorObjectParseStatus::Supported: return "Supported";
    case EditorObjectParseStatus::Partial: return "Partial";
    default: return "Malformed";
    }
}

bool NormalizeHistoricalObjectReference(std::string_view input,
    std::string& normalized, std::string* reason)
{
    normalized.clear();
    if (input.empty()) { if (reason) *reason = "Reference is empty."; return false; }
    if (input.front() == '/' || input.front() == '\\') {
        if (reason) *reason = "Absolute references are not allowed."; return false;
    }
    std::string current;
    std::vector<std::string> components;
    auto finish = [&]() -> bool {
        if (current.empty() || current == "." || current == "..") return false;
        components.push_back(current); current.clear(); return true;
    };
    for (unsigned char ch : input)
    {
        if (ch < 0x20 || ch == 0x7f || ch == ':') {
            if (reason) *reason = "Reference contains a control character or colon.";
            return false;
        }
        if (ch == '/' || ch == '\\') {
            if (!finish()) { if (reason) *reason = "Reference has an unsafe path component."; return false; }
        } else current.push_back(static_cast<char>(std::tolower(ch)));
    }
    if (!finish()) { if (reason) *reason = "Reference has an unsafe path component."; return false; }
    if (EndsWithObject(components.back()))
        components.back().resize(components.back().size() - 7);
    if (components.back().empty()) { if (reason) *reason = "Reference has no object name."; return false; }
    for (std::size_t i = 0; i < components.size(); ++i) {
        if (i) normalized.push_back('\\');
        normalized += components[i];
    }
    return true;
}

bool ValidateHistoricalObjectReference(std::string_view input,
    std::string* reason)
{
    std::string ignored;
    return NormalizeHistoricalObjectReference(input, ignored, reason);
}

std::vector<const EditorObjectLibraryEntry*> EditorObjectLibrary::Find(
    std::string_view normalizedReference) const
{
    std::vector<const EditorObjectLibraryEntry*> result;
    const auto found = index_.find(std::string(normalizedReference));
    if (found == index_.end()) return result;
    result.reserve(found->second.size());
    for (std::size_t index : found->second) result.push_back(&entries_[index]);
    return result;
}

void EditorObjectLibrary::Clear()
{
    loaded_ = false; root_.clear(); entries_.clear(); index_.clear();
}

void EditorObjectLibrary::RebuildIndex()
{
    index_.clear();
    for (std::size_t i = 0; i < entries_.size(); ++i)
        index_[entries_[i].referenceId].push_back(i);
}
