# wxSDKEditor camera controls

The wx-only viewport uses RMB free-look. Drag right looks right and drag up
looks up. The old vertical behavior was inverted because downward-growing
window Y was added to a pitch convention that the camera projection negates.
Horizontal yaw was already correct.

`View > Camera Controls` stores independent horizontal/vertical inversion and
mouse sensitivity in `wxConfig`; these user preferences never alter a document,
snapshot, history, or repository file. RMB capture anchors its first motion,
clamps pitch to +/-89 degrees, wraps yaw, hides/restores the cursor, and is
released on button-up, focus loss, destruction, and shutdown.

W/S moves on the camera's horizontal forward vector and A/D on its horizontal
right vector. Q/E remains world-down/world-up, preventing pitch from causing
unintentional vertical flight.
