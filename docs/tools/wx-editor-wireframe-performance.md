# wxSDKEditor Wireframe Performance

The renderer coarse-culls instances using transformed object bounds before it
requests geometry. Selected instances are stable-prioritized, then model order
is preserved. Default frame limits are 512 visible instances, 150,000 submitted
triangles, and 450,000 semantic line segments. Truncation is deterministic and
reported in the viewport overlay; it never causes a document mutation.

The bounded overlay reports visible and culled instances, decoded assets,
submitted triangles, drawn lines, fallback bounds, budget-skipped objects, and
decode failures without writing per-frame Output logs.

The production headless sample considered at most 64 instances per scene and
10,000 triangles/30,000 lines per scene. Across 17 scenes it observed 197
visible instances, 113 decoded assets, 137,069 submitted triangles, 129,317
clipped lines, 853 coarse-culled instances, two legitimate fallback instances,
165 budget-skipped objects, and zero decode failures.

