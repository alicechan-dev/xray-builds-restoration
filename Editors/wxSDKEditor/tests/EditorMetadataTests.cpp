#include "editor_app/EditorDocument.h"
#include "editor_assets/EditorAssetSelectionModel.h"
#include "editor_assets/EditorMetadataCatalogAdapter.h"
#include "editor_assets/EditorMetadataLoader.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace
{
class TempRoot
{
public:
    TempRoot()
    {
        path = std::filesystem::temp_directory_path() /
            ("xr_wx_metadata_" + std::to_string(
                std::chrono::steady_clock::now().time_since_epoch().count()));
        std::filesystem::create_directories(path);
    }
    ~TempRoot()
    {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }
    void Write(const std::filesystem::path& relative, const std::string& text)
    {
        const std::filesystem::path target = path / relative;
        std::filesystem::create_directories(target.parent_path());
        std::ofstream output(target, std::ios::binary);
        output << text;
    }
    std::filesystem::path path;
};
}

int RunEditorMetadataTests()
{
    int failures = 0;
    const auto check = [&failures](bool condition, const char* message) {
        if (condition) return;
        ++failures;
        std::cerr << "FAIL: metadata " << message << '\n';
    };

    EditorMetadataLoader loader;
    EditorImportedMetadata parsed;
    std::string reason;
    check(loader.ParseMetadataText(
        "; comment\n[Weapon]\nclass = W_AK74 ; inline\n"
        "$spawn = weapons\\ak-74\nmusic\\amb01\n", "memory.ltx",
        parsed, &reason) && parsed.sections.size() == 1 &&
        parsed.sections[0].name == "weapon" &&
        parsed.sections[0].entries.size() == 3 &&
        parsed.sections[0].FindFirst("CLASS")->value == "W_AK74",
        "basic grammar, comments, lowercase section, and bare entries");

    EditorImportedMetadata malformed;
    check(loader.ParseMetadataText(
        "[s]\n=value\nkey=one\nkey=two\n", "malformed.ltx",
        malformed, &reason) && malformed.sections[0].entries.size() == 2 &&
        malformed.diagnostics.size() == 2,
        "empty keys warn and duplicate keys are retained with diagnostics");
    check(!loader.ParseMetadataText("[broken\nkey=value\n", "bad.ltx",
        malformed, &reason) && reason.find("bad.ltx:1") != std::string::npos,
        "malformed section fails with source-aware reason");
    check(!loader.ParseMetadataText("[same]\nkey=1\n[SAME]\nkey=2\n",
        "duplicate.ltx", malformed, &reason),
        "duplicate sections fail case-insensitively");
    check(loader.ParseMetadataText("[child]:parent\nkey=1\n",
        "suffix.ltx", malformed, &reason) &&
        malformed.diagnostics.size() == 1 &&
        malformed.diagnostics[0].sourceLine == 1,
        "unaudited section suffix is ignored with file-line diagnostic");

    TempRoot root;
    root.Write("system.ltx", "#include \"sub/one.ltx\"\n[root]\nkey=1\n");
    root.Write("sub/one.ltx", "#include \"two.ltx\"\n[one]\nkey=1\n");
    root.Write("sub/two.ltx", "[two]\n$spawn = test\\two\nclass = TEST\n");
    EditorImportedMetadata loaded;
    check(loader.LoadMetadataRoot(root.path, "system.ltx", loaded, &reason) &&
        loaded.files.size() == 3 && loaded.includesFollowed == 2 &&
        loaded.sections.size() == 3 && loaded.sections[0].name == "two" &&
        loaded.sections[1].name == "one" && loaded.sections[2].name == "root",
        "nested includes load inline in deterministic order");

    root.Write("cycle-a.ltx", "#include \"cycle-b.ltx\"\n");
    root.Write("cycle-b.ltx", "#include \"cycle-a.ltx\"\n");
    EditorImportedMetadata preserved;
    preserved.sections.push_back({"preserved", "", 0, {}});
    check(!loader.LoadMetadataRoot(root.path, "cycle-a.ltx", preserved,
        &reason) && preserved.sections.size() == 1 &&
        preserved.sections[0].name == "preserved",
        "include cycle fails atomically");

    const std::filesystem::path outside = root.path.parent_path() /
        (root.path.filename().string() + "_outside.ltx");
    { std::ofstream output(outside); output << "[outside]\nkey=1\n"; }
    root.Write("traversal.ltx", "#include \"../" +
        outside.filename().string() + "\"\n");
    check(!loader.LoadMetadataRoot(root.path, "traversal.ltx", loaded,
        &reason), "include traversal outside root is rejected");
    root.Write("absolute.ltx", "#include \"" +
        outside.generic_string() + "\"\n");
    check(!loader.LoadMetadataRoot(root.path, "absolute.ltx", loaded,
        &reason), "absolute include is rejected");
    std::error_code cleanupError;
    std::filesystem::remove(outside, cleanupError);

    EditorMetadataLimits depthLimits;
    depthLimits.maxIncludeDepth = 1;
    check(!EditorMetadataLoader(depthLimits).LoadMetadataRoot(
        root.path, "system.ltx", loaded, &reason),
        "include depth limit is enforced");
    EditorMetadataLimits fileLimits;
    fileLimits.maxFiles = 2;
    check(!EditorMetadataLoader(fileLimits).LoadMetadataRoot(
        root.path, "system.ltx", loaded, &reason),
        "file-count limit is enforced");
    EditorMetadataLimits byteLimits;
    byteLimits.maxBytesPerFile = 8;
    byteLimits.maxTotalBytes = 16;
    check(!EditorMetadataLoader(byteLimits).LoadMetadataRoot(
        root.path, "system.ltx", loaded, &reason),
        "input-size limits are enforced");

    EditorMetadataCatalogResult catalogResult =
        BuildEditorMetadataCatalog(parsed);
    check(catalogResult.catalog.Entries().size() == 1 &&
        catalogResult.unsupportedSections == 0 &&
        catalogResult.catalog.Entries()[0].id == "imported.section.weapon" &&
        !catalogResult.catalog.Entries()[0].placeable &&
        catalogResult.catalog.Entries()[0].sourceFile == "memory.ltx" &&
        catalogResult.catalog.Entries()[0].sourceLine == 2,
        "$spawn section becomes namespaced non-placeable descriptor with provenance");
    EditorImportedMetadata unsupported;
    unsupported.sections.push_back({"sound", "sound.ltx", 1,
        {{"class", "SOUND", "sound.ltx", 2}}});
    const EditorMetadataCatalogResult unsupportedResult =
        BuildEditorMetadataCatalog(unsupported);
    check(unsupportedResult.catalog.Entries().empty() &&
        unsupportedResult.unsupportedSections == 1 &&
        EditorAssetCatalog::CreateBuiltIn().Entries().size() == 6,
        "unsupported raw section is counted and synthetic catalog is unaffected");

    EditorImportedMetadata duplicates;
    duplicates.sections.push_back({"a b", "one.ltx", 1,
        {{"$spawn", "one", "one.ltx", 2}}});
    duplicates.sections.push_back({"a_b", "two.ltx", 1,
        {{"$spawn", "two", "two.ltx", 2}}});
    catalogResult = BuildEditorMetadataCatalog(duplicates);
    check(catalogResult.catalog.Entries().size() == 1 &&
        catalogResult.diagnostics.size() == 1,
        "normalized imported ID collision is diagnosed and skipped");

    const EditorAssetCatalog synthetic = EditorAssetCatalog::CreateBuiltIn();
    EditorAssetSelectionModel selection;
    check(selection.Select(synthetic, "demo.actor") &&
        !selection.Select(catalogResult.catalog, "imported.section.a_b") &&
        selection.SelectedId() == "demo.actor",
        "non-placeable import activation cannot replace synthetic selection");
    EditorDocument document;
    BuildEditorMetadataCatalog(parsed);
    check(!document.IsModified(),
        "metadata parsing and catalog conversion do not dirty document");
    return failures;
}
