# wxSDKEditor Tool Modes

`EditorToolController` is a wx-free state holder for Select, Move, Place Object,
and Place Light. Mode changes do not mutate `EditorDocument` or command history.
Select enables ordinary preview picking; Move additionally exposes the X/Z
gizmo; placement modes convert viewport clicks into synthetic transforms.

The frame owns a conventional wx toolbar using built-in art: Select, Move,
Object, Light, Undo, Redo, Delete, Frame, and Snap. The first four tools are
mutually exclusive. Menu and toolbar check/enabled state follow the presenter,
history, and viewport snap state. Existing WASD/QE camera keys are preserved.

Input priority is active gizmo drag, Move-mode gizmo handle, placement click,
ordinary preview pick, then empty space. Right mouse remains camera navigation.
Escape cancels a gizmo drag first; otherwise it leaves placement/Move mode and
returns to Select without touching history.

This is an experimental shell boundary, not a port of historical VCL tools.

`PlaceAsset` generalizes placement for the selected synthetic catalog
descriptor. The legacy Object and Light commands remain available and route to
`demo.physic_object` and `demo.point_light` metadata while preserving their
familiar naming behavior. Asset Browser activation selects a descriptor and
enters `PlaceAsset`; Escape returns to Select without committing a command.
