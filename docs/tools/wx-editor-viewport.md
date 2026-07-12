# wxSDKEditor Viewport Host

`wxSDKEditor` now has a dedicated viewport host ready for later renderer
integration without linking any historical render target.

## Architecture And Ownership

The wx-free `EditorViewportController` owns `EditorViewportState`: dimensions,
focus, pointer/buttons, a small placeholder camera, grid visibility, and
deterministic tick counters. It accepts an injected `IEditorViewportRenderer`.

`wxEditorViewport` owns both the controller and a
`NullEditorViewportRenderer`. The null backend receives resize/render calls but
creates no device and draws nothing. The wx control handles platform events and
uses buffered 2D painting for the current fallback. `MainFrame` owns the wx
control and only invokes grid, reset, and focus commands.

## Current Surface

The non-dockable wxAUI center pane paints a dark neutral background, an optional
2D grid, center axes, and diagnostic text for size, mouse/focus state, camera
values, and the explicit message `Renderer is not connected`. It remains
always visible after layout reset.

Development-only input proves event flow:

* right-button drag changes placeholder yaw and pitch;
* wheel adjusts movement speed between documented bounds;
* focused W/S move Z, A/D move X, and Q/E move Y;
* focus loss clears held movement state;
* View -> Viewport Grid toggles the grid;
* View -> Reset Viewport Camera restores `(0, 1, -5)`, zero angles, and speed 5;
* View -> Focus Viewport transfers keyboard focus.

This is not scene camera logic. It has no picking, gizmos, assets, matrices,
graphics API, swap chain, or X-Ray renderer calls.

Headless tests cover resize clamping and renderer notification, focus and
pointer transitions, right drag, wheel speed, focused/unfocused movement,
deterministic ticks, grid toggling, camera reset, and render-boundary
invocation. Tests-only configuration includes no wx headers and does not
discover or link wxWidgets.

Future integration should implement `IEditorViewportRenderer` in a separate,
target-local adapter while preserving the state/controller contract. Old
`xrEProps`, LevelEditor, and runtime renderer targets remain untouched.

The first concrete consumer is now the wx-free `EditorPreviewRenderer`. It
converts a derived synthetic preview scene into semantic screen-space draw-list
primitives; `wxEditorViewport` alone maps those primitives to wx colors. See
[wxSDKEditor Preview Scene](wx-editor-preview-scene.md).
