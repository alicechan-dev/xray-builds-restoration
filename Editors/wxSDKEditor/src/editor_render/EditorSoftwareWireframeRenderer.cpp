#include "editor_render/EditorSoftwareWireframeRenderer.h"

#include "editor_render/EditorRenderAssetRegistry.h"
#include "editor_render/EditorRenderGeometryCache.h"
#include "editor_render/EditorRenderScene.h"
#include "editor_render/EditorStaticMeshGeometry.h"
#include "editor_view/EditorViewportState.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <unordered_set>

namespace
{
constexpr float Pi = 3.14159265358979323846f;

struct Point3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Point2
{
    float x = 0.0f;
    float y = 0.0f;
};

float DegreesToRadians(float value) { return value * Pi / 180.0f; }

Point3 TransformPosition(const EditorGeometryPosition& point,
    const EditorTransform& transform)
{
    Point3 result{point.x * transform.sx, point.y * transform.sy,
        point.z * transform.sz};
    const float cr = std::cos(transform.roll);
    const float sr = std::sin(transform.roll);
    result = {result.x * cr - result.y * sr,
        result.x * sr + result.y * cr, result.z};
    const float cp = std::cos(transform.pitch);
    const float sp = std::sin(transform.pitch);
    result = {result.x, result.y * cp - result.z * sp,
        result.y * sp + result.z * cp};
    const float cy = std::cos(transform.yaw);
    const float sy = std::sin(transform.yaw);
    result = {result.x * cy + result.z * sy, result.y,
        -result.x * sy + result.z * cy};
    result.x += transform.x;
    result.y += transform.y;
    result.z += transform.z;
    return result;
}

Point3 ToView(const Point3& world, const EditorWireframeCamera& camera)
{
    const float dx = world.x - camera.x;
    const float dy = world.y - camera.y;
    const float dz = world.z - camera.z;
    const float yaw = DegreesToRadians(camera.yawDegrees);
    const float pitch = DegreesToRadians(camera.pitchDegrees);
    const float cy = std::cos(yaw);
    const float sy = std::sin(yaw);
    const float x1 = cy * dx - sy * dz;
    const float z1 = sy * dx + cy * dz;
    const float cp = std::cos(pitch);
    const float sp = std::sin(pitch);
    return {x1, cp * dy - sp * z1, sp * dy + cp * z1};
}

bool Project(const Point3& point, const EditorWireframeCamera& camera,
    Point2& projected)
{
    if (point.z < camera.nearPlane || point.z > camera.farPlane)
        return false;
    const float focal = static_cast<float>(camera.viewportHeight) * 0.5f /
        std::tan(DegreesToRadians(camera.verticalFovDegrees) * 0.5f);
    projected.x = static_cast<float>(camera.viewportWidth) * 0.5f +
        point.x * focal / point.z;
    projected.y = static_cast<float>(camera.viewportHeight) * 0.5f -
        point.y * focal / point.z;
    return std::isfinite(projected.x) && std::isfinite(projected.y);
}

bool ClipDepth(Point3& a, Point3& b, float nearPlane, float farPlane)
{
    if ((a.z < nearPlane && b.z < nearPlane) ||
        (a.z > farPlane && b.z > farPlane))
        return false;
    const auto clip = [&a, &b](float plane, bool keepGreater)
    {
        const bool aOutside = keepGreater ? a.z < plane : a.z > plane;
        const bool bOutside = keepGreater ? b.z < plane : b.z > plane;
        if (aOutside == bOutside) return;
        const float denominator = b.z - a.z;
        if (std::fabs(denominator) < 1.0e-12f) return;
        const float t = (plane - a.z) / denominator;
        const Point3 intersection{a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t, plane};
        if (aOutside) a = intersection; else b = intersection;
    };
    clip(nearPlane, true);
    clip(farPlane, false);
    return a.z >= nearPlane && b.z >= nearPlane &&
        a.z <= farPlane && b.z <= farPlane;
}

bool ClipScreen(Point2& a, Point2& b, float width, float height)
{
    float t0 = 0.0f;
    float t1 = 1.0f;
    const float dx = b.x - a.x;
    const float dy = b.y - a.y;
    const std::array<float, 4> p{-dx, dx, -dy, dy};
    const std::array<float, 4> q{a.x, width - a.x, a.y, height - a.y};
    for (std::size_t i = 0; i < p.size(); ++i)
    {
        if (std::fabs(p[i]) < 1.0e-12f)
        {
            if (q[i] < 0.0f) return false;
            continue;
        }
        const float value = q[i] / p[i];
        if (p[i] < 0.0f) t0 = (std::max)(t0, value);
        else t1 = (std::min)(t1, value);
        if (t0 > t1) return false;
    }
    const Point2 original = a;
    a = {original.x + dx * t0, original.y + dy * t0};
    b = {original.x + dx * t1, original.y + dy * t1};
    return true;
}

bool CoarselyVisible(const EditorRenderInstance& instance,
    const EditorWireframeCamera& camera)
{
    if (!instance.visible || !instance.objectBounds.valid ||
        !instance.transform.IsFinite())
        return false;
    const auto& bounds = instance.objectBounds;
    const EditorGeometryPosition localCenter{
        (bounds.minX + bounds.maxX) * 0.5f,
        (bounds.minY + bounds.maxY) * 0.5f,
        (bounds.minZ + bounds.maxZ) * 0.5f};
    const Point3 center = ToView(TransformPosition(localCenter,
        instance.transform), camera);
    const float ex = (bounds.maxX - bounds.minX) * 0.5f *
        std::fabs(instance.transform.sx);
    const float ey = (bounds.maxY - bounds.minY) * 0.5f *
        std::fabs(instance.transform.sy);
    const float ez = (bounds.maxZ - bounds.minZ) * 0.5f *
        std::fabs(instance.transform.sz);
    const float radius = std::sqrt(ex * ex + ey * ey + ez * ez);
    if (center.z + radius < camera.nearPlane ||
        center.z - radius > camera.farPlane)
        return false;
    const float tanY = std::tan(DegreesToRadians(camera.verticalFovDegrees) * 0.5f);
    const float tanX = tanY * static_cast<float>(camera.viewportWidth) /
        static_cast<float>(camera.viewportHeight);
    const float depth = (std::max)(center.z, camera.nearPlane);
    return std::fabs(center.x) <= depth * tanX + radius &&
        std::fabs(center.y) <= depth * tanY + radius;
}

bool AddEdge(const Point3& first, const Point3& second,
    const EditorWireframeCamera& camera, bool selected,
    EditorWireframeFrame& frame, const EditorWireframeBudget& budget)
{
    if (frame.lines.size() >= budget.maximumLines)
        return false;
    Point3 a = first;
    Point3 b = second;
    if (!ClipDepth(a, b, camera.nearPlane, camera.farPlane)) return true;
    Point2 p0, p1;
    if (!Project(a, camera, p0) || !Project(b, camera, p1)) return true;
    if (!ClipScreen(p0, p1, static_cast<float>(camera.viewportWidth - 1),
            static_cast<float>(camera.viewportHeight - 1)))
        return true;
    frame.lines.push_back({p0.x, p0.y, p1.x, p1.y, selected});
    return true;
}
}

bool EditorWireframeCamera::IsValid() const
{
    return viewportWidth > 0 && viewportHeight > 0 &&
        std::isfinite(x) && std::isfinite(y) && std::isfinite(z) &&
        std::isfinite(yawDegrees) && std::isfinite(pitchDegrees) &&
        std::isfinite(verticalFovDegrees) && verticalFovDegrees > 1.0f &&
        verticalFovDegrees < 179.0f && std::isfinite(nearPlane) &&
        std::isfinite(farPlane) && nearPlane > 0.0f && farPlane > nearPlane;
}

EditorWireframeCamera MakeEditorWireframeCamera(
    const EditorViewportState& state)
{
    EditorWireframeCamera camera;
    camera.x = state.camera.x;
    camera.y = state.camera.y;
    camera.z = state.camera.z;
    camera.yawDegrees = state.camera.yaw;
    camera.pitchDegrees = -state.camera.pitch;
    camera.viewportWidth = state.width;
    camera.viewportHeight = state.height;
    return camera;
}

EditorWireframeFrame EditorSoftwareWireframeRenderer::Render(
    const EditorRenderScene& scene, EditorRenderAssetRegistry& assets,
    EditorRenderGeometryCache& cache, const EditorWireframeCamera& camera,
    bool backfaceCulling, const EditorWireframeBudget& budget) const
{
    EditorWireframeFrame frame;
    frame.validCamera = camera.IsValid();
    if (!frame.validCamera || budget.maximumInstances == 0 ||
        budget.maximumTriangles == 0 || budget.maximumLines == 0)
        return frame;

    std::vector<const EditorRenderInstance*> ordered;
    ordered.reserve(scene.Instances().size());
    for (const auto& instance : scene.Instances())
        if (instance.selected) ordered.push_back(&instance);
    for (const auto& instance : scene.Instances())
        if (!instance.selected) ordered.push_back(&instance);
    std::unordered_set<std::string> decodedAssets;

    for (const auto* instance : ordered)
    {
        ++frame.statistics.consideredInstances;
        if (!CoarselyVisible(*instance, camera))
        {
            ++frame.statistics.culledInstances;
            continue;
        }
        if (frame.statistics.visibleInstances >= budget.maximumInstances)
        {
            ++frame.statistics.budgetSkippedObjects;
            frame.budgetExceeded = true;
            continue;
        }
        ++frame.statistics.visibleInstances;
        EditorRenderObjectAsset* asset = assets.Find(instance->assetId);
        if (!asset || asset->objectKind != EditorObjectKind::Static)
        {
            ++frame.statistics.fallbackBounds;
            continue;
        }
        std::string decodeReason;
        const EditorStaticAssetGeometry* geometry =
            cache.Request(*asset, &decodeReason);
        if (!geometry)
        {
            ++frame.statistics.decodeFailures;
            ++frame.statistics.fallbackBounds;
            continue;
        }
        decodedAssets.insert(asset->assetId);
        bool objectBudgetReached = false;
        for (const auto& mesh : geometry->meshes)
        {
            for (const auto& triangle : mesh.buffer.triangles)
            {
                if (frame.statistics.trianglesSubmitted >=
                        budget.maximumTriangles ||
                    frame.lines.size() + 3 > budget.maximumLines)
                {
                    objectBudgetReached = true;
                    frame.budgetExceeded = true;
                    break;
                }
                const Point3 a = ToView(TransformPosition(
                    mesh.buffer.positions[triangle.a], instance->transform), camera);
                const Point3 b = ToView(TransformPosition(
                    mesh.buffer.positions[triangle.b], instance->transform), camera);
                const Point3 c = ToView(TransformPosition(
                    mesh.buffer.positions[triangle.c], instance->transform), camera);
                if (backfaceCulling && a.z >= camera.nearPlane &&
                    b.z >= camera.nearPlane && c.z >= camera.nearPlane)
                {
                    Point2 pa, pb, pc;
                    if (Project(a, camera, pa) && Project(b, camera, pb) &&
                        Project(c, camera, pc))
                    {
                        const float area = (pb.x - pa.x) * (pc.y - pa.y) -
                            (pb.y - pa.y) * (pc.x - pa.x);
                        if (area >= 0.0f) continue;
                    }
                }
                ++frame.statistics.trianglesSubmitted;
                AddEdge(a, b, camera, instance->selected, frame, budget);
                AddEdge(b, c, camera, instance->selected, frame, budget);
                AddEdge(c, a, camera, instance->selected, frame, budget);
            }
            if (objectBudgetReached) break;
        }
        if (objectBudgetReached)
            ++frame.statistics.budgetSkippedObjects;
    }
    frame.statistics.decodedAssets = decodedAssets.size();
    frame.statistics.linesDrawn = frame.lines.size();
    return frame;
}
