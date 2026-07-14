#include "editor_assets/EditorObjectLibraryResolver.h"

const char* ToString(EditorObjectResolutionState value)
{
    switch (value) {
    case EditorObjectResolutionState::LibraryNotLoaded: return "Library Not Loaded";
    case EditorObjectResolutionState::Resolved: return "Resolved";
    case EditorObjectResolutionState::Missing: return "Missing";
    case EditorObjectResolutionState::Ambiguous: return "Ambiguous";
    default: return "Invalid";
    }
}

EditorObjectResolutionResult ResolveObjectReference(
    const EditorObjectLibrary& library, std::string_view reference)
{
    EditorObjectResolutionResult result;
    if (!library.IsLoaded()) { result.reason = "No Object Library is loaded."; return result; }
    if (!NormalizeHistoricalObjectReference(reference, result.normalizedQuery,
        &result.reason)) { result.state = EditorObjectResolutionState::Invalid; return result; }
    const auto candidates = library.Find(result.normalizedQuery);
    result.candidateCount = candidates.size();
    if (candidates.empty()) {
        result.state = EditorObjectResolutionState::Missing;
        result.reason = "No library entry matches the normalized reference.";
    } else if (candidates.size() > 1) {
        result.state = EditorObjectResolutionState::Ambiguous;
        result.reason = "Multiple library entries share the normalized reference.";
    } else {
        result.state = EditorObjectResolutionState::Resolved;
        result.entry = candidates.front();
        result.matchedReferenceId = result.entry->referenceId;
    }
    return result;
}

EditorObjectLibraryStatistics BuildEditorObjectLibraryStatistics(
    const EditorObjectLibrary& library)
{
    EditorObjectLibraryStatistics result;
    result.entries = library.Entries().size();
    for (const auto& entry : library.Entries()) {
        if (entry.parseStatus == EditorObjectParseStatus::Supported) ++result.supported;
        else if (entry.parseStatus == EditorObjectParseStatus::Partial) ++result.partial;
        else ++result.malformed;
        if (entry.kind == EditorObjectKind::Static) ++result.staticObjects;
        else if (entry.kind == EditorObjectKind::Skeletal) ++result.skeletalObjects;
        else ++result.unknownObjects;
    }
    for (const auto& entry : library.Entries())
        if (library.Find(entry.referenceId).size() > 1) ++result.duplicateReferences;
    return result;
}

void Accumulate(EditorSceneObjectResolutionStatistics& s,
    EditorObjectResolutionState state)
{
    ++s.queried;
    switch (state) {
    case EditorObjectResolutionState::Resolved: ++s.resolved; break;
    case EditorObjectResolutionState::Missing: ++s.missing; break;
    case EditorObjectResolutionState::Ambiguous: ++s.ambiguous; break;
    case EditorObjectResolutionState::Invalid: ++s.invalid; break;
    default: ++s.libraryNotLoaded; break;
    }
}
