#include "editor_assets/EditorObjectLibraryLoader.h"
#include "editor_assets/EditorObjectLibraryResolver.h"
#include "editor_scene/EditorHistoricalSceneProbe.h"
#include "editor_scene/EditorHistoricalSceneDocument.h"
#include "editor_model/EditorPropertySet.h"
#include "editor_model/EditorTreeModel.h"
#include "editor_model/EditorTreeSnapshot.h"
#include "editor_render/EditorRenderAssetRegistry.h"
#include "editor_render/EditorRenderScene.h"
#include "editor_assets/EditorStaticMeshDecoder.h"
#include "editor_render/EditorRenderGeometryCache.h"
#include "editor_render/EditorSoftwareWireframeRenderer.h"
#include "editor_view/EditorTreePreviewAdapter.h"
#include "editor_view/EditorViewportController.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_set>
#include <vector>

namespace
{
using Bytes = std::vector<unsigned char>;
void U16(Bytes& out, std::uint16_t value) { out.push_back(value & 255); out.push_back(value >> 8); }
void U32(Bytes& out, std::uint32_t value) { for (int i=0;i<4;++i) out.push_back((value >> (i*8)) & 255); }
void F32(Bytes& out, float value) { std::uint32_t bits=0; std::memcpy(&bits,&value,4); U32(out,bits); }
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
    Bytes mesh, meshPayload;
    U16(meshPayload,0x0011); Chunk(mesh,0x1000,meshPayload); meshPayload.clear();
    Z(meshPayload,"mesh"); Chunk(mesh,0x1001,meshPayload); meshPayload.clear();
    F32(meshPayload,-1);F32(meshPayload,-2);F32(meshPayload,-3);
    F32(meshPayload,1);F32(meshPayload,2);F32(meshPayload,3);
    Chunk(mesh,0x1004,meshPayload);meshPayload.clear();
    U32(meshPayload,3);for(int i=0;i<9;++i)F32(meshPayload,0);meshPayload.push_back(0);meshPayload.push_back(0);meshPayload.push_back(0);
    Chunk(mesh,0x1005,meshPayload);meshPayload.clear();
    U32(meshPayload,1);for(int i=0;i<6;++i)U32(meshPayload,0);Chunk(mesh,0x1006,meshPayload);meshPayload.clear();
    U32(meshPayload,3);meshPayload.push_back(0);meshPayload.push_back(0);meshPayload.push_back(0);Chunk(mesh,0x1008,meshPayload);meshPayload.clear();
    U16(meshPayload,1);Z(meshPayload,"surface");U32(meshPayload,1);U32(meshPayload,0);Chunk(mesh,0x1009,meshPayload);meshPayload.clear();
    U32(meshPayload,1);Z(meshPayload,"Texture");meshPayload.push_back(2);meshPayload.push_back(0);meshPayload.push_back(0);U32(meshPayload,3);
    for(int i=0;i<6;++i)F32(meshPayload,0);for(int i=0;i<3;++i)U32(meshPayload,i);Chunk(mesh,0x1012,meshPayload);
    Chunk(payload,0,mesh); Chunk(body,0x0910,payload);
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
        failures += Check(entry.bounds.valid && entry.meshes.size()==1 &&
            entry.meshes[0].vertexCount==3 && entry.meshes[0].triangleCount==1 &&
            entry.meshes[0].supported, "mesh metadata and bounds");
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
    EditorRenderAssetRegistry registry;
    failures += Check(registry.Build(library,&reason),"registry build");
    const auto* renderAsset=registry.Find("buildings\\house");
    failures += Check(renderAsset && renderAsset->readiness==
        EditorRenderAssetReadiness::StaticGeometryDecodeCandidate &&
        renderAsset->totalVertices==3 && renderAsset->totalTriangles==1,
        "render asset readiness");
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
        &propertyResolution, renderAsset);
    failures += Check(properties.Find("object_library.resolution") &&
        properties.Find("object_library.matched_id"), "resolution properties");
    failures += Check(properties.Find("object_library.render_asset") &&
        properties.Find("object_library.real_bounds"), "render asset properties");
    const auto preview=BuildEditorPreviewScene(model,node.Path(),&registry);
    const auto* previewObject=preview.FindByLogicalPath(node.Path());
    failures += Check(previewObject&&previewObject->realBounds&&previewObject->sizeX>1.0f&&
        previewObject->selected,"resolved asset preview bounds and selection");
    const auto renderScene=BuildEditorRenderScene(model,registry,node.Path());
    failures += Check(renderScene.Instances().size()==1&&
        !renderScene.Instances()[0].fallback&&renderScene.Instances()[0].selected&&
        renderScene.Instances()[0].logicalPath==node.Path(),"renderer-neutral submission");
    EditorSceneManifest historicalManifest;
    historicalManifest.hasVersion = true;
    historicalManifest.version = 5;
    historicalManifest.sourceFile = "fixture.level";
    EditorSceneObjectRecord historicalObject;
    historicalObject.recordIndex = 7;
    historicalObject.sourceOffset = 100;
    historicalObject.hasClassId = true;
    historicalObject.classId = 2;
    historicalObject.hasName = true;
    historicalObject.name = "house_0000";
    historicalObject.hasTransform = true;
    historicalObject.scale = {1.0f, 1.0f, 1.0f};
    historicalObject.bodyDecode.status =
        EditorHistoricalObjectDecodeStatus::Supported;
    historicalObject.bodyDecode.hasSceneObject = true;
    historicalObject.bodyDecode.sceneObject.referenceName = "Buildings/House";
    historicalManifest.objects.push_back(historicalObject);
    EditorHistoricalSceneDocument historicalDocument;
    failures += Check(historicalDocument.BuildFromManifest(
        std::move(historicalManifest), &reason), "read-only historical document");
    const std::string stableId = historicalDocument.Objects().front().stableRecordId;
    const auto historicalRenderScene = BuildHistoricalRenderScene(
        historicalDocument, registry, stableId);
    failures += Check(historicalRenderScene.Instances().size() == 1 &&
        historicalRenderScene.Instances()[0].logicalPath == stableId &&
        historicalRenderScene.Instances()[0].selected &&
        historicalRenderScene.Instances()[0].assetId == "buildings\\house" &&
        historicalRenderScene.Instances()[0].objectBounds.valid &&
        !historicalRenderScene.Instances()[0].fallback,
        "read-only historical scene submits resolved render asset bounds");
    registry.Clear();
    const auto fallbackPreview=BuildEditorPreviewScene(model,node.Path(),&registry);
    failures += Check(fallbackPreview.FindByLogicalPath(node.Path())&&
        !fallbackPreview.FindByLogicalPath(node.Path())->realBounds,
        "registry clear restores fallback preview");
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
    EditorRenderAssetRegistry registry;
    if(!registry.Build(library,&reason)){std::cerr<<"Registry build failed: "<<reason<<'\n';return 2;}
    const auto rs=registry.Statistics();
    std::size_t geometryDecoded=0,geometryUnsupported=0,geometryMalformed=0;
    std::size_t decodedVertices=0,decodedTriangles=0,degenerateTriangles=0;
    std::size_t zeroAreaTriangles=0,boundsMismatches=0,invalidIndices=0;
    std::size_t nonFinitePositions=0,totalGeometryBytes=0,peakAssetBytes=0;
    EditorStaticMeshDecoderLimits auditLimits;
    auditLimits.maximumDecodedBytes=512ull*1024ull*1024ull;
    EditorStaticMeshDecoder geometryDecoder(auditLimits);
    for(const auto& asset:registry.Assets()){
        if(asset.readiness!=EditorRenderAssetReadiness::StaticGeometryDecodeCandidate)continue;
        EditorStaticAssetGeometry geometry;std::string decodeReason;
        const auto status=geometryDecoder.Decode(library.Root(),asset,geometry,&decodeReason);
        if(status==EditorStaticGeometryDecodeStatus::Decoded){++geometryDecoded;
            decodedVertices+=geometry.totalVertices;decodedTriangles+=geometry.totalTriangles;
            degenerateTriangles+=geometry.degenerateTriangles;zeroAreaTriangles+=geometry.zeroAreaTriangles;
            boundsMismatches+=geometry.boundsMismatches;totalGeometryBytes+=geometry.MemoryBytes();
            peakAssetBytes=(std::max)(peakAssetBytes,geometry.MemoryBytes());}
        else if(status==EditorStaticGeometryDecodeStatus::Unsupported)++geometryUnsupported;
        else{++geometryMalformed;invalidIndices+=decodeReason.find("out-of-range")!=std::string::npos;
            nonFinitePositions+=decodeReason.find("non-finite")!=std::string::npos;}}
    std::size_t staticObjects=0,skeletalObjects=0,totalMeshes=0,totalVertices=0,totalTriangles=0;
    std::size_t minMeshes=library.Entries().empty()?0:static_cast<std::size_t>(-1),maxMeshes=0;
    std::size_t minVertices=static_cast<std::size_t>(-1),maxVertices=0,minTriangles=static_cast<std::size_t>(-1),maxTriangles=0;
    std::size_t malformedMeshes=0,unknownMeshChunks=0,sgMeshes=0,vmap0=0,vmap1=0,vmap2=0,uvMaps=0,weightMaps=0;
    std::uint64_t largestMeshPayload=0;std::map<std::uint16_t,std::size_t> meshVersions;
    std::map<std::string,std::size_t> meshDiagnostics;
    std::map<std::uint32_t,std::size_t> unknownMeshIds;
    for(const auto& e:library.Entries()){
        staticObjects+=e.kind==EditorObjectKind::Static;skeletalObjects+=e.kind==EditorObjectKind::Skeletal;
        minMeshes=(std::min)(minMeshes,e.meshes.size());maxMeshes=(std::max)(maxMeshes,e.meshes.size());totalMeshes+=e.meshes.size();
        for(const auto& m:e.meshes){++meshVersions[m.version];totalVertices+=m.vertexCount;totalTriangles+=m.triangleCount;
            minVertices=(std::min)(minVertices,m.vertexCount);maxVertices=(std::max)(maxVertices,m.vertexCount);
            minTriangles=(std::min)(minTriangles,m.triangleCount);maxTriangles=(std::max)(maxTriangles,m.triangleCount);
            malformedMeshes+=m.malformed;unknownMeshChunks+=m.unknownChunkCount;sgMeshes+=m.smoothingGroupsPresent;
            vmap0+=m.vmapFormat==0;vmap1+=m.vmapFormat==1;vmap2+=m.vmapFormat==2;uvMaps+=m.uvMapCount;weightMaps+=m.weightMapCount;
            largestMeshPayload=(std::max)(largestMeshPayload,m.sourcePayloadSize);
            for(const auto& d:m.diagnostics)++meshDiagnostics[d];for(auto id:m.unknownChunkIds)++unknownMeshIds[id];}}
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
    std::cout<<"static_objects="<<staticObjects<<"\nskeletal_objects="<<skeletalObjects
        <<"\ntotal_meshes="<<totalMeshes<<"\nmeshes_per_object_min="<<minMeshes<<"\nmeshes_per_object_max="<<maxMeshes
        <<"\ntotal_vertices="<<totalVertices<<"\nvertices_min="<<(minVertices==static_cast<std::size_t>(-1)?0:minVertices)
        <<"\nvertices_max="<<maxVertices<<"\ntotal_triangles="<<totalTriangles
        <<"\ntriangles_min="<<(minTriangles==static_cast<std::size_t>(-1)?0:minTriangles)<<"\ntriangles_max="<<maxTriangles
        <<"\nmalformed_meshes="<<malformedMeshes<<"\nunknown_mesh_chunks="<<unknownMeshChunks
        <<"\nsmoothing_group_meshes="<<sgMeshes<<"\nvmap0_meshes="<<vmap0<<"\nvmap1_meshes="<<vmap1<<"\nvmap2_meshes="<<vmap2
        <<"\nuv_maps="<<uvMaps<<"\nweight_maps="<<weightMaps<<"\nlargest_mesh_payload="<<largestMeshPayload<<'\n';
    for(const auto& v:meshVersions)std::cout<<"mesh_version_"<<v.first<<'='<<v.second<<'\n';
    for(const auto& d:meshDiagnostics)std::cout<<"mesh_diagnostic="<<d.first<<" count="<<d.second<<'\n';
    for(const auto& id:unknownMeshIds)std::cout<<"unknown_mesh_chunk_0x"<<std::hex<<id.first<<std::dec<<'='<<id.second<<'\n';
    std::cout<<"registry_assets="<<rs.assets<<"\nbounds_only="<<rs.boundsOnly<<"\nmetadata_ready="<<rs.metadataReady
        <<"\ndecode_candidates="<<rs.decodeCandidates<<"\nskeletal_deferred="<<rs.skeletalDeferred
        <<"\nasset_unsupported="<<rs.unsupported<<"\nasset_malformed="<<rs.malformed<<'\n';
    std::cout<<"geometry_decoded_assets="<<geometryDecoded
        <<"\ngeometry_unsupported_assets="<<geometryUnsupported
        <<"\ngeometry_malformed_assets="<<geometryMalformed
        <<"\ndecoded_vertices="<<decodedVertices<<"\ndecoded_triangles="<<decodedTriangles
        <<"\ninvalid_indices="<<invalidIndices<<"\nnon_finite_positions="<<nonFinitePositions
        <<"\ndegenerate_triangles="<<degenerateTriangles<<"\nzero_area_triangles="<<zeroAreaTriangles
        <<"\nbounds_mismatches="<<boundsMismatches<<"\ndecoded_memory_bytes="<<totalGeometryBytes
        <<"\npeak_asset_geometry_bytes="<<peakAssetBytes<<'\n';
    std::size_t boundsInstances=0,fallbackInstances=0,partialReferences=0;
    std::size_t sampleVisible=0,sampleDecoded=0,sampleTriangles=0,sampleLines=0;
    std::size_t sampleCulled=0,sampleFallback=0,sampleBudgetSkipped=0,sampleFailures=0;
    EditorRenderGeometryCache sampleCache;sampleCache.Bind(library.Root(),&registry);
    EditorSoftwareWireframeRenderer sampleRenderer;
    std::unordered_set<std::string> allUsedAssets;
    std::size_t largestWorkingSet=0,largestWorkingSetBytes=0;
    for (const auto& scene : scenes) {
        EditorSceneManifest manifest;
        if (!EditorHistoricalSceneProbe().ProbeSceneFile(scene, manifest, &reason)) {
            std::cerr << "Scene probe failed for " << scene.filename().string() << ": " << reason << '\n';
            return 2;
        }
        EditorSceneObjectResolutionStatistics perScene;
        EditorRenderScene sampleScene;
        std::unordered_set<std::string> sceneAssets;
        std::size_t sceneStatic=0,sceneSkeletal=0,sceneMissingAssets=0;
        std::size_t sceneEstimatedGpuBytes=0;
        for (const auto& object : manifest.objects) {
            if (!object.bodyDecode.hasSceneObject) continue;
            const auto result = ResolveObjectReference(
                library, object.bodyDecode.sceneObject.referenceName);
            Accumulate(perScene, result.state); Accumulate(aggregate, result.state);
            if(result.entry&&result.entry->parseStatus==EditorObjectParseStatus::Partial)++partialReferences;
            const auto* asset=result.entry?registry.Find(result.entry->referenceId):nullptr;
            if (asset && sceneAssets.insert(asset->assetId).second) {
                allUsedAssets.insert(asset->assetId);
                if(asset->objectKind==EditorObjectKind::Static){++sceneStatic;
                    sceneEstimatedGpuBytes+=asset->totalVertices*sizeof(EditorGeometryPosition)+
                        asset->totalTriangles*3*sizeof(std::uint32_t);}
                else if(asset->objectKind==EditorObjectKind::Skeletal)++sceneSkeletal;
            } else if (!asset && result.state==EditorObjectResolutionState::Missing)
                ++sceneMissingAssets;
            if(asset&&asset->bounds.valid)++boundsInstances;else ++fallbackInstances;
            if(asset&&asset->bounds.valid&&object.hasTransform&&sampleScene.Instances().size()<64){
                EditorRenderInstance instance;instance.assetId=asset->assetId;instance.objectBounds=asset->bounds;
                instance.readiness=asset->readiness;instance.fallback=false;
                instance.transform.x=object.position[0];instance.transform.y=object.position[1];instance.transform.z=object.position[2];
                instance.transform.pitch=object.rotation[0];instance.transform.yaw=object.rotation[1];instance.transform.roll=object.rotation[2];
                instance.transform.sx=object.scale[0];instance.transform.sy=object.scale[1];instance.transform.sz=object.scale[2];
                sampleScene.Add(std::move(instance));}
        }
        if(!sampleScene.Instances().empty()){
            EditorWireframeCamera camera;camera.viewportWidth=800;camera.viewportHeight=600;
            camera.x=sampleScene.Instances().front().transform.x;
            camera.y=sampleScene.Instances().front().transform.y;
            camera.z=sampleScene.Instances().front().transform.z-5.0f;
            EditorWireframeBudget budget;budget.maximumInstances=64;budget.maximumTriangles=10000;budget.maximumLines=30000;
            const auto frame=sampleRenderer.Render(sampleScene,registry,sampleCache,camera,false,budget);
            sampleVisible+=frame.statistics.visibleInstances;sampleDecoded+=frame.statistics.decodedAssets;
            sampleTriangles+=frame.statistics.trianglesSubmitted;sampleLines+=frame.statistics.linesDrawn;
            sampleCulled+=frame.statistics.culledInstances;sampleFallback+=frame.statistics.fallbackBounds;
            sampleBudgetSkipped+=frame.statistics.budgetSkippedObjects;sampleFailures+=frame.statistics.decodeFailures;}
        largestWorkingSet=(std::max)(largestWorkingSet,sceneAssets.size());
        largestWorkingSetBytes=(std::max)(largestWorkingSetBytes,sceneEstimatedGpuBytes);
        std::cout << "scene=" << scene.filename().string() << " queried=" << perScene.queried
            << " resolved=" << perScene.resolved << " missing=" << perScene.missing
            << " ambiguous=" << perScene.ambiguous << " invalid=" << perScene.invalid
            << " unique_assets=" << sceneAssets.size() << " static_assets=" << sceneStatic
            << " skeletal_deferred=" << sceneSkeletal << " missing_assets=" << sceneMissingAssets
            << " estimated_gpu_bytes=" << sceneEstimatedGpuBytes << '\n';
    }
    std::cout << "scenes=" << scenes.size() << "\nreferences_queried=" << aggregate.queried
        << "\nresolved=" << aggregate.resolved << "\nmissing=" << aggregate.missing
        << "\nambiguous=" << aggregate.ambiguous << "\ninvalid=" << aggregate.invalid << '\n';
    std::cout<<"unique_assets_used_across_scenes="<<allUsedAssets.size()
        <<"\nunused_library_assets="<<(registry.Assets().size()-allUsedAssets.size())
        <<"\nlargest_scene_working_set="<<largestWorkingSet
        <<"\nlargest_scene_estimated_gpu_bytes="<<largestWorkingSetBytes<<'\n';
    std::cout<<"scene_instances_with_bounds="<<boundsInstances<<"\nscene_fallback_instances="<<fallbackInstances
        <<"\npartial_entry_references="<<partialReferences<<'\n';
    std::cout<<"wireframe_sample_visible_instances="<<sampleVisible
        <<"\nwireframe_sample_decoded_assets="<<sampleDecoded
        <<"\nwireframe_sample_triangles="<<sampleTriangles
        <<"\nwireframe_sample_lines="<<sampleLines
        <<"\nwireframe_sample_culled_instances="<<sampleCulled
        <<"\nwireframe_sample_fallback_bounds="<<sampleFallback
        <<"\nwireframe_sample_budget_skipped="<<sampleBudgetSkipped
        <<"\nwireframe_sample_decode_failures="<<sampleFailures<<'\n';
    return 0;
}

int DiagnoseFramedObject(const std::filesystem::path& libraryRoot,
    const std::filesystem::path& sceneFile, const std::string& objectName)
{
    EditorObjectLibrary library;
    EditorObjectLibraryLoadStatistics load;
    std::string reason;
    if (!EditorObjectLibraryLoader().Load(libraryRoot, library, load, &reason))
    {
        std::cerr << "Object Library load failed: " << reason << '\n';
        return 2;
    }
    EditorRenderAssetRegistry registry;
    if (!registry.Build(library, &reason))
    {
        std::cerr << "Registry build failed: " << reason << '\n';
        return 2;
    }
    EditorSceneManifest manifest;
    if (!EditorHistoricalSceneProbe().ProbeSceneFile(sceneFile, manifest, &reason))
    {
        std::cerr << "Scene probe failed: " << reason << '\n';
        return 2;
    }
    const EditorSceneObjectRecord* selected = nullptr;
    for (const EditorSceneObjectRecord& object : manifest.objects)
        if (object.hasName && object.name == objectName)
        {
            selected = &object;
            break;
        }
    if (!selected || !selected->hasTransform ||
        !selected->bodyDecode.hasSceneObject)
    {
        std::cerr << "Named transformed SceneObject was not found.\n";
        return 2;
    }
    const auto resolution = ResolveObjectReference(library,
        selected->bodyDecode.sceneObject.referenceName);
    const EditorRenderObjectAsset* asset = resolution.entry
        ? registry.Find(resolution.entry->referenceId) : nullptr;
    if (!asset || !asset->bounds.valid)
    {
        std::cerr << "Selected reference did not resolve to valid bounds.\n";
        return 2;
    }
    EditorRenderInstance instance;
    instance.logicalPath = selected->name;
    instance.assetId = asset->assetId;
    instance.objectBounds = asset->bounds;
    instance.readiness = asset->readiness;
    instance.fallback = false;
    instance.selected = true;
    instance.transform.x = selected->position[0];
    instance.transform.y = selected->position[1];
    instance.transform.z = selected->position[2];
    instance.transform.pitch = selected->rotation[0];
    instance.transform.yaw = selected->rotation[1];
    instance.transform.roll = selected->rotation[2];
    instance.transform.sx = selected->scale[0];
    instance.transform.sy = selected->scale[1];
    instance.transform.sz = selected->scale[2];
    EditorRenderScene scene;
    scene.Add(instance);
    EditorRenderGeometryCache cache;
    cache.Bind(library.Root(), &registry);
    EditorSoftwareWireframeRenderer renderer;
    EditorWireframeCamera before;
    before.viewportWidth = 557;
    before.viewportHeight = 774;
    before.x = instance.transform.x;
    before.y = 1.0f;
    before.z = instance.transform.z;
    const EditorWireframeFrame beforeFrame = renderer.Render(scene, registry,
        cache, before, false);
    const EditorWireframeWorldBounds worldBounds =
        ComputeEditorWireframeWorldBounds(instance);
    EditorViewportController controller;
    controller.OnResize(557, 774);
    controller.FrameCameraOn(worldBounds.center.x, worldBounds.center.y,
        worldBounds.center.z, worldBounds.radius);
    const EditorWireframeCamera after =
        MakeEditorWireframeCamera(controller.State());
    const EditorWireframeFrame afterFrame = renderer.Render(scene, registry,
        cache, after, false);
    const auto print = [](const char* prefix, const EditorWireframeCamera& camera,
        const EditorWireframeFrame& frame)
    {
        const auto& d = frame.selectedDiagnostic;
        std::cout << prefix << "_logical_path=" << d.logicalPath
            << '\n' << prefix << "_asset_id=" << d.assetId
            << '\n' << prefix << "_resolved=" << d.assetResolved
            << '\n' << prefix << "_readiness=" << ToString(d.readiness)
            << '\n' << prefix << "_transform=" << d.transform.x << ','
            << d.transform.y << ',' << d.transform.z
            << '\n' << prefix << "_object_bounds=" << d.objectBounds.minX << ','
            << d.objectBounds.minY << ',' << d.objectBounds.minZ << ':'
            << d.objectBounds.maxX << ',' << d.objectBounds.maxY << ','
            << d.objectBounds.maxZ
            << '\n' << prefix << "_world_bounds=" << d.worldBounds.minimum.x << ','
            << d.worldBounds.minimum.y << ',' << d.worldBounds.minimum.z << ':'
            << d.worldBounds.maximum.x << ',' << d.worldBounds.maximum.y << ','
            << d.worldBounds.maximum.z
            << '\n' << prefix << "_world_center=" << d.worldBounds.center.x << ','
            << d.worldBounds.center.y << ',' << d.worldBounds.center.z
            << '\n' << prefix << "_camera=" << camera.x << ',' << camera.y << ','
            << camera.z << ',' << camera.yawDegrees << ',' << camera.pitchDegrees
            << '\n' << prefix << "_basis_right=" << d.cameraBasis.right.x << ','
            << d.cameraBasis.right.y << ',' << d.cameraBasis.right.z
            << '\n' << prefix << "_basis_up=" << d.cameraBasis.up.x << ','
            << d.cameraBasis.up.y << ',' << d.cameraBasis.up.z
            << '\n' << prefix << "_basis_forward=" << d.cameraBasis.forward.x << ','
            << d.cameraBasis.forward.y << ',' << d.cameraBasis.forward.z
            << '\n' << prefix << "_camera_center=" << d.cameraSpaceCenter.x << ','
            << d.cameraSpaceCenter.y << ',' << d.cameraSpaceCenter.z
            << '\n' << prefix << "_near_far=" << camera.nearPlane << ','
            << camera.farPlane
            << '\n' << prefix << "_cull=" << ToString(d.cullReason)
            << '\n' << prefix << "_visible=" << frame.statistics.visibleInstances
            << '\n' << prefix << "_decoded=" << frame.statistics.decodedAssets
            << '\n' << prefix << "_triangles=" << frame.statistics.trianglesSubmitted
            << '\n' << prefix << "_lines=" << frame.statistics.linesDrawn << '\n';
    };
    print("before", before, beforeFrame);
    print("after", after, afterFrame);
    return afterFrame.statistics.visibleInstances > 0 &&
        afterFrame.statistics.decodedAssets > 0 &&
        afterFrame.statistics.trianglesSubmitted > 0 &&
        afterFrame.statistics.linesDrawn > 0 ? 0 : 3;
}
