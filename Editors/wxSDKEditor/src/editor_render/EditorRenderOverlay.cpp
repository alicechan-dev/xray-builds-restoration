#include "editor_render/EditorRenderOverlay.h"

#include "editor_render/EditorRenderScene.h"
#include "editor_view/EditorPreviewScene.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <unordered_map>

namespace
{
constexpr float Pi = 3.14159265358979323846f;

EditorRenderPoint3D Transform(EditorRenderPoint3D point,
    const EditorTransform& transform)
{
    point.x *= transform.sx; point.y *= transform.sy; point.z *= transform.sz;
    const float cr=std::cos(transform.roll), sr=std::sin(transform.roll);
    point = {point.x*cr-point.y*sr, point.x*sr+point.y*cr, point.z};
    const float cp=std::cos(transform.pitch), sp=std::sin(transform.pitch);
    point = {point.x, point.y*cp-point.z*sp, point.y*sp+point.z*cp};
    const float cy=std::cos(transform.yaw), sy=std::sin(transform.yaw);
    point = {point.x*cy+point.z*sy, point.y, -point.x*sy+point.z*cy};
    point.x += transform.x; point.y += transform.y; point.z += transform.z;
    return point;
}

void AddLine(EditorRenderOverlayBatch& batch,
    const EditorRenderOverlayOptions& options, EditorRenderPoint3D first,
    EditorRenderPoint3D second, EditorRenderOverlayStyle style,
    bool depthTest, bool selected = false)
{
    if (batch.lines.size() >= options.maximumLines)
    { ++batch.skippedLines; return; }
    batch.lines.push_back({first, second, style, depthTest, selected});
}

void AddCross(EditorRenderOverlayBatch& batch,
    const EditorRenderOverlayOptions& options, EditorRenderPoint3D center,
    float radius, EditorRenderOverlayStyle style, bool depth, bool selected)
{
    AddLine(batch, options, {center.x-radius,center.y,center.z},
        {center.x+radius,center.y,center.z}, style, depth, selected);
    AddLine(batch, options, {center.x,center.y-radius,center.z},
        {center.x,center.y+radius,center.z}, style, depth, selected);
    AddLine(batch, options, {center.x,center.y,center.z-radius},
        {center.x,center.y,center.z+radius}, style, depth, selected);
}

void AddRing(EditorRenderOverlayBatch& batch,
    const EditorRenderOverlayOptions& options, EditorRenderPoint3D center,
    float radius, EditorRenderOverlayStyle style, bool depth, bool selected)
{
    constexpr int Segments = 24;
    for (int i=0; i<Segments; ++i)
    {
        const float a=2.0f*Pi*i/Segments, b=2.0f*Pi*(i+1)/Segments;
        AddLine(batch, options,
            {center.x+std::cos(a)*radius,center.y,center.z+std::sin(a)*radius},
            {center.x+std::cos(b)*radius,center.y,center.z+std::sin(b)*radius},
            style, depth, selected);
    }
}

void AddBounds(EditorRenderOverlayBatch& batch,
    const EditorRenderOverlayOptions& options, const EditorRenderInstance& item)
{
    if (!item.objectBounds.valid || !item.transform.IsFinite()) return;
    const auto& b=item.objectBounds;
    std::array<EditorRenderPoint3D,8> p{{
        {b.minX,b.minY,b.minZ},{b.maxX,b.minY,b.minZ},
        {b.maxX,b.maxY,b.minZ},{b.minX,b.maxY,b.minZ},
        {b.minX,b.minY,b.maxZ},{b.maxX,b.minY,b.maxZ},
        {b.maxX,b.maxY,b.maxZ},{b.minX,b.maxY,b.maxZ}}};
    for (auto& value:p) value=Transform(value,item.transform);
    constexpr std::array<std::array<int,2>,12> edges{{
        {{0,1}},{{1,2}},{{2,3}},{{3,0}},{{4,5}},{{5,6}},{{6,7}},{{7,4}},
        {{0,4}},{{1,5}},{{2,6}},{{3,7}}}};
    for (const auto& edge:edges)
        AddLine(batch,options,p[edge[0]],p[edge[1]],
            item.selected?EditorRenderOverlayStyle::Selected:
                EditorRenderOverlayStyle::Object,
            !item.selected,item.selected);
}
}

EditorRenderOverlayBatch BuildEditorRenderOverlay(
    const EditorPreviewScene& previewScene,
    const EditorRenderScene& renderScene,
    const EditorRenderFrameContext& frame,
    const EditorRenderOverlayOptions& options)
{
    EditorRenderOverlayBatch batch;
    std::unordered_map<std::string,const EditorRenderInstance*> instances;
    for (const auto& value:renderScene.Instances()) instances[value.logicalPath]=&value;

    if (options.grid)
    {
        constexpr int Half=20;
        for (int i=-Half;i<=Half;++i)
        {
            const auto style=i==0?EditorRenderOverlayStyle::Selected:
                EditorRenderOverlayStyle::Grid;
            AddLine(batch,options,{static_cast<float>(-Half),0.0f,static_cast<float>(i)},
                {static_cast<float>(Half),0.0f,static_cast<float>(i)},style,true);
            AddLine(batch,options,{static_cast<float>(i),0.0f,static_cast<float>(-Half)},
                {static_cast<float>(i),0.0f,static_cast<float>(Half)},style,true);
        }
    }

    std::vector<EditorRenderLabel> labels;
    std::size_t order=0;
    for (const auto& object:previewScene.GetObjects())
    {
        if (!object.visible) { ++order; continue; }
        const bool runtime=object.kind==EditorPreviewKind::Spawn ||
            object.kind==EditorPreviewKind::Marker;
        if ((runtime&&!options.runtimeMarkers) ||
            (object.kind==EditorPreviewKind::Glow&&!options.glowMarkers) ||
            ((object.kind==EditorPreviewKind::Light ||
              object.kind==EditorPreviewKind::HistoricalLight)&&!options.lightMarkers) ||
            (object.kind==EditorPreviewKind::Unknown&&!options.unsupportedBounds))
        { ++order; continue; }
        const EditorRenderPoint3D center{object.x,object.y,object.z};
        const bool depth=runtime?options.depthTestRuntimeMarkers:true;
        EditorRenderOverlayStyle style=EditorRenderOverlayStyle::Runtime;
        if(object.kind==EditorPreviewKind::Glow) style=EditorRenderOverlayStyle::Glow;
        else if(object.kind==EditorPreviewKind::Light ||
            object.kind==EditorPreviewKind::HistoricalLight)
            style=EditorRenderOverlayStyle::Light;
        else if(object.kind==EditorPreviewKind::Unknown)
            style=EditorRenderOverlayStyle::Unsupported;

        const auto found=instances.find(object.logicalPath);
        if (options.objectBounds && found!=instances.end())
            AddBounds(batch,options,*found->second);
        else if (object.kind==EditorPreviewKind::Light ||
            object.kind==EditorPreviewKind::Glow ||
            object.kind==EditorPreviewKind::HistoricalLight)
            AddRing(batch,options,center,(std::max)(0.15f,object.sizeX*0.5f),
                style,depth,object.selected);
        else
            AddCross(batch,options,center,0.25f,
                object.selected?EditorRenderOverlayStyle::Selected:style,
                object.selected?false:depth,object.selected);

        if (object.selected)
            AddCross(batch,options,center,0.4f,
                EditorRenderOverlayStyle::Selected,false,true);
        if (options.gizmo && object.selected)
        {
            AddLine(batch,options,center,{center.x+1.0f,center.y,center.z},
                EditorRenderOverlayStyle::GizmoX,false,true);
            AddLine(batch,options,center,{center.x,center.y,center.z+1.0f},
                EditorRenderOverlayStyle::GizmoZ,false,true);
        }

        const bool wantsLabel=options.labels==EditorPreviewLabelPolicy::All ||
            (options.labels==EditorPreviewLabelPolicy::SelectedOnly&&object.selected);
        if (wantsLabel)
        {
            const EditorProjectedPoint projected=ProjectEditorWorldPoint(center,frame);
            if(projected.finite&&projected.inFront&&projected.insideDepth&&
                projected.insideViewport)
                labels.push_back({object.label,projected.screenX+10.0f,
                    projected.screenY-8.0f,projected.depth,object.selected,order});
        }
        ++order;
    }
    std::stable_sort(labels.begin(),labels.end(),[](const auto& a,const auto& b){
        if(a.selected!=b.selected) return a.selected>b.selected;
        if(a.depth!=b.depth) return a.depth<b.depth;
        return a.stableOrder<b.stableOrder;
    });
    if(labels.size()>options.maximumLabels)
    { batch.skippedLabels=labels.size()-options.maximumLabels;
      labels.resize(options.maximumLabels); }
    batch.labels=std::move(labels);
    return batch;
}
