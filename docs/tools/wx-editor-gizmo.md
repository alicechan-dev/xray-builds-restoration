# wxSDKEditor Move Gizmo

The synthetic viewport exposes a wx-free `EditorMoveGizmo`. A selected object
receives semantic X and Z lines. X maps horizontally and Z vertically at the
existing 40 pixels-per-unit preview scale; Y, rotation, and scale are deferred.

Input priority is active drag, axis handle, preview object, then empty space.
Motion changes only the viewport preview copy. Release calls the presenter once
and creates at most one transform command. Escape restores the starting preview
and creates no command. Right drag remains the placeholder camera gesture.

**View -> Snap Move To Grid** enables fixed 1.0-unit X/Z snapping. It is off by
default and does not affect property edits. Gizmo math and tests use no wx types;
wx code only translates events and paints semantic primitives.

This is a synthetic top-down prototype, not a real 3D gizmo, geometry snap,
physics tool, or historical SDK transform binding.
