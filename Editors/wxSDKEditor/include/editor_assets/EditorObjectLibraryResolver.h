#ifndef XR_WX_SDK_EDITOR_EDITOR_OBJECT_LIBRARY_RESOLVER_H
#define XR_WX_SDK_EDITOR_EDITOR_OBJECT_LIBRARY_RESOLVER_H

#include "editor_assets/EditorObjectLibrary.h"

#include <cstddef>
#include <string>
#include <string_view>

enum class EditorObjectResolutionState
{
    LibraryNotLoaded,
    Resolved,
    Missing,
    Ambiguous,
    Invalid
};

struct EditorObjectResolutionResult
{
    EditorObjectResolutionState state =
        EditorObjectResolutionState::LibraryNotLoaded;
    std::string normalizedQuery;
    std::string matchedReferenceId;
    std::size_t candidateCount = 0;
    const EditorObjectLibraryEntry* entry = nullptr;
    std::string reason;
};

struct EditorObjectLibraryStatistics
{
    std::size_t entries = 0;
    std::size_t supported = 0;
    std::size_t partial = 0;
    std::size_t malformed = 0;
    std::size_t staticObjects = 0;
    std::size_t skeletalObjects = 0;
    std::size_t unknownObjects = 0;
    std::size_t duplicateReferences = 0;
};

struct EditorSceneObjectResolutionStatistics
{
    std::size_t queried = 0;
    std::size_t resolved = 0;
    std::size_t missing = 0;
    std::size_t ambiguous = 0;
    std::size_t invalid = 0;
    std::size_t libraryNotLoaded = 0;
};

const char* ToString(EditorObjectResolutionState value);
EditorObjectResolutionResult ResolveObjectReference(
    const EditorObjectLibrary& library, std::string_view historicalReference);
EditorObjectLibraryStatistics BuildEditorObjectLibraryStatistics(
    const EditorObjectLibrary& library);
void Accumulate(EditorSceneObjectResolutionStatistics& statistics,
    EditorObjectResolutionState state);

#endif
