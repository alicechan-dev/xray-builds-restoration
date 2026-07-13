#ifndef XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_SCENE_PROBE_H
#define XR_WX_SDK_EDITOR_EDITOR_HISTORICAL_SCENE_PROBE_H

#include "editor_scene/EditorSceneManifest.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

struct EditorSceneProbeLimits
{
    std::size_t maximumFileSize = 256u * 1024u * 1024u;
    std::size_t maximumChunks = 250000;
    std::size_t maximumNestingDepth = 32;
    std::size_t maximumObjects = 100000;
    std::size_t maximumStringLength = 4096;
    std::size_t maximumDiagnostics = 256;
};

class EditorHistoricalSceneProbe
{
public:
    explicit EditorHistoricalSceneProbe(
        EditorSceneProbeLimits limits = {});

    bool ProbeSceneFile(const std::filesystem::path& path,
        EditorSceneManifest& result, std::string* reason = nullptr) const;
    bool ProbeSceneBytes(const std::vector<std::uint8_t>& bytes,
        std::string sourceName, EditorSceneManifest& result,
        std::string* reason = nullptr) const;

private:
    EditorSceneProbeLimits limits_;
};

#endif
