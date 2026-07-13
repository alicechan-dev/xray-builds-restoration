# wxSDKEditor Transform Model

`EditorTransform` is a wx-free value owned by each `EditorTreeNode`. It stores
position, yaw/pitch/roll, and scale; defaults are zero position/rotation and
unit scale. The model rejects non-finite values. Object-like nodes expose
editable `position.x`, `position.y`, and `position.z`.

Transforms are document state. Snapshot v3 writes all nine values; v1 and v2
remain readable and receive defaults. A malformed v3 transform rejects the
whole load without replacing the current model. Rename and reparent preserve
the node value, while delete/undo restores it through model snapshots.

The preview adapter maps model position directly. Demo objects receive explicit
deterministic positions; imported and legacy nodes use the default origin.
Property and gizmo commits use command history and normal dirty/save points.

This is development-model state only. It is not bound to historical X-Ray
objects, real level formats, assets, `xrEProps`, or LevelEditor.

Synthetic placement supplies the initial transform through the same validated
model API; no additional snapshot field or format revision is required.

Asset descriptors may provide a default transform. Placement replaces only
the prospective X/Z coordinates (with optional snap) and preserves descriptor
defaults such as Point Light Y=1. Snapshot v4 retains the same nine transform
values and adds only the optional asset ID field.
