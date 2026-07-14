#include "editor_assets/EditorObjectLibraryLoader.h"
#include "editor_assets/EditorObjectLibraryResolver.h"
#include "editor_scene/EditorHistoricalSceneProbe.h"
#include "editor_model/EditorPropertySet.h"
#include "editor_model/EditorTreeModel.h"
#include "editor_model/EditorTreeSnapshot.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace
{
using Bytes = std::vector<unsigned char>;
void U16(Bytes& out, std::uint16_t value) { out.push_back(value & 255); out.push_back(value >> 8); }
void U32(Bytes& out, std::uint32_t value) { for (int i=0;i<4;++i) out.push_back((value >> (i*8)) & 255); }
void Z(Bytes& out, const char* value) { while (*value) out.push_back(*value++); out.push_back(0); }
void Chunk(Bytes& out, std::uint32_t id, const Bytes& payload)
{ U32(out,id); U32(out,static_cast<std::uint32_t>(payload.size())); out.insert(out.end(),payload.begin(),payload.end()); }
Bytes ValidObject()
{
    Bytes body, payload;
    U16(payload,0x0010); Chunk(body,0x0900,payload); payload.clear();
    U32(payload,0); Chunk(body,0x0903,payload); payload.clear();
    U32(payload,17); U32(payload,0); Chunk(body,0x0911,payload); payload.clear();
    U32(payload,1); Z(payload,"surface"); Z(payload,"models\\model");
    Z(payload,"default"); Z(payload,"materials\\default");
    Z(payload,"textures\\brick"); Z(payload,"Texture");
    U32(payload,0); U32(payload,0); U32(payload,1); Chunk(body,0x0907,payload); payload.clear();
    Bytes mesh; Chunk(payload,0,mesh); Chunk(body,0x0910,payload);
    Bytes file; Chunk(file,0x7777,body); return file;
}
bool Write(const std::filesystem::path& path, const Bytes& bytes)
{
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream(path,std::ios::binary); stream.write(
        reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return !!stream;
}
int Check(bool condition, const char* message)
{
    if (condition) return 0; std::cerr << "FAIL object library: " << message << '\n'; return 1;
}
}

int RunEditorObjectLibraryTests()
{
    int failures = 0;
    std::string normalized, reason;
    failures += Check(NormalizeHistoricalObjectReference(
        "Buildings/Bar/House.OBJECT", normalized, &reason) &&
        normalized == "buildings\\bar\\house", "normalization");
    failures += Check(!ValidateHistoricalObjectReference("../evil.object"), "traversal rejected");
    failures += Check(!ValidateHistoricalObjectReference("C:\\evil.object"), "drive path rejected");
    failures += Check(!ValidateHistoricalObjectReference("a//b.object"), "empty component rejected");

    const auto root = std::filesystem::temp_directory_path() /
        ("xr_object_library_test_" + std::to_string(
            std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);
    EditorObjectLibrary library;
    EditorObjectLibraryLoadStatistics statistics;
    EditorObjectLibraryLoader loader;
    failures += Check(loader.Load(root, library, statistics, &reason) &&
        library.IsLoaded() && library.Entries().empty(), "empty root");
    failures += Check(Write(root / "Buildings" / "House.object", ValidObject()), "write fixture");
    failures += Check(loader.Load(root, library, statistics, &reason), "load fixture");
    failures += Check(library.Entries().size() == 1, "entry count");
    if (!library.Entries().empty()) {
        const auto& entry = library.Entries().front();
        failures += Check(entry.referenceId == "buildings\\house", "stable reference");
        failures += Check(entry.version == 0x0010 && entry.meshCount == 1 &&
            entry.surfaceCount == 1, "confirmed metadata");
        failures += Check(entry.textureReferences.size() == 1 &&
            entry.shaderReferences.size() == 2 && entry.materialReferences.size() == 1,
            "inert surface references");
    }
    auto resolved = ResolveObjectReference(library,"BUILDINGS\\HOUSE.object");
    failures += Check(resolved.state == EditorObjectResolutionState::Resolved,
        "resolved reference");
    failures += Check(ResolveObjectReference(library,"missing").state ==
        EditorObjectResolutionState::Missing, "missing reference");
    failures += Check(ResolveObjectReference(library,"..\\bad").state ==
        EditorObjectResolutionState::Invalid, "invalid reference");
    EditorTreeModel model;
    auto& modelRoot = model.CreateRoot("Scene");
    auto& node = model.AddChild(modelRoot, "house", "historical",
        EditorItemKind::Object);
    EditorHistoricalOriginMetadata origin;
    origin.sourceSceneName = "fixture.level";
    origin.sourceSceneVersion = 5;
    origin.sourceClassId = 2;
    origin.sourceObjectIndex = 1;
    origin.sourceOffset = 100;
    origin.sourceName = "house";
    origin.sourceStableRecordId = "historical.object.1";
    origin.decodeStatus = "Supported";
    origin.sourceTransformConfirmed = true;
    origin.referenceName = "Buildings/House";
    model.SetNodeHistoricalOrigin(node, origin);
    std::string before, after;
    failures += Check(SerializeEditorTreeSnapshot(model, before, &reason),
        "snapshot before resolution");
    const auto propertyResolution = ResolveObjectReference(library,
        node.HistoricalOrigin()->referenceName);
    const auto properties = BuildEditorNodePropertySet(node, nullptr,
        &propertyResolution);
    failures += Check(properties.Find("object_library.resolution") &&
        properties.Find("object_library.matched_id"), "resolution properties");
    failures += Check(SerializeEditorTreeSnapshot(model, after, &reason) &&
        before == after, "session resolution leaves snapshot unchanged");
#ifndef _WIN32
    Write(root / "buildings" / "HOUSE.object", ValidObject());
    failures += Check(loader.Load(root, library, statistics, &reason), "load collision");
    failures += Check(ResolveObjectReference(library,"buildings/house").state ==
        EditorObjectResolutionState::Ambiguous, "case-insensitive collision");
#endif
    Write(root / "broken.object", Bytes{1,2,3});
    failures += Check(loader.Load(root, library, statistics, &reason) &&
        statistics.malformed == 1, "malformed retained");

    EditorObjectLibrary previous = library;
    EditorObjectLibraryLimits limits; limits.maximumFiles = 1;
    EditorObjectLibraryLoader limited(limits);
    failures += Check(!limited.Load(root, library, statistics, &reason), "file limit");
    failures += Check(library.Entries().size() == previous.Entries().size(), "atomic replacement");
    std::error_code ignored; std::filesystem::remove_all(root,ignored);
    return failures;
}

int AuditEditorObjectLibrary(const std::filesystem::path& libraryRoot,
    const std::filesystem::path& sceneRoot)
{
    EditorObjectLibrary library;
    EditorObjectLibraryLoadStatistics load;
    std::string reason;
    if (!EditorObjectLibraryLoader().Load(libraryRoot, library, load, &reason)) {
        std::cerr << "Object Library audit load failed: " << reason << '\n'; return 2;
    }
    std::vector<std::filesystem::path> scenes;
    std::error_code error;
    for (std::filesystem::recursive_directory_iterator it(sceneRoot, error), end;
        !error && it != end; it.increment(error))
        if (it->is_regular_file(error) && it->path().extension() == ".level")
            scenes.push_back(it->path());
    if (error) { std::cerr << "Scene enumeration failed: " << error.message() << '\n'; return 2; }
    std::sort(scenes.begin(), scenes.end());
    EditorSceneObjectResolutionStatistics aggregate;
    std::cout << "library_files=" << load.filesScanned << "\nentries=" << load.entriesLoaded
        << "\nsupported=" << load.supported << "\npartial=" << load.partial
        << "\nmalformed=" << load.malformed << "\nduplicates=" << load.duplicateReferences
        << "\nsource_bytes=" << load.sourceBytes << "\nmetadata_bytes_read=" << load.bytesRead << '\n';
    for (const auto& scene : scenes) {
        EditorSceneManifest manifest;
        if (!EditorHistoricalSceneProbe().ProbeSceneFile(scene, manifest, &reason)) {
            std::cerr << "Scene probe failed for " << scene.filename().string() << ": " << reason << '\n';
            return 2;
        }
        EditorSceneObjectResolutionStatistics perScene;
        for (const auto& object : manifest.objects) {
            if (!object.bodyDecode.hasSceneObject) continue;
            const auto result = ResolveObjectReference(
                library, object.bodyDecode.sceneObject.referenceName);
            Accumulate(perScene, result.state); Accumulate(aggregate, result.state);
        }
        std::cout << "scene=" << scene.filename().string() << " queried=" << perScene.queried
            << " resolved=" << perScene.resolved << " missing=" << perScene.missing
            << " ambiguous=" << perScene.ambiguous << " invalid=" << perScene.invalid << '\n';
    }
    std::cout << "scenes=" << scenes.size() << "\nreferences_queried=" << aggregate.queried
        << "\nresolved=" << aggregate.resolved << "\nmissing=" << aggregate.missing
        << "\nambiguous=" << aggregate.ambiguous << "\ninvalid=" << aggregate.invalid << '\n';
    return 0;
}
