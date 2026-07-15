# D3D11 Device Lifecycle

`wxEditorViewport` owns the concrete D3D11 backend. The backend lazily creates
its device and swap chain after the wx child control has a valid native HWND.
The presenter and document never own GPU resources.

- Hardware device creation is attempted first with BGRA support.
- Debug builds request the debug layer but retry without it when unavailable.
- WARP is the initialization fallback.
- Zero-sized/minimized viewports retain no render/depth target work.
- Resize releases target views before `ResizeBuffers` and recreates color and
  depth views afterward.
- Switching to Software Diagnostic releases D3D resources immediately.
- The destroy event stops the timer and releases the swap chain before HWND
  destruction.
- A render/resize failure falls back once; there is no recreate loop.

Background erase is suppressed because D3D clears the swap-chain target. wx
semantic overlays are drawn after `Present`; this is the current explicit
interop boundary and may move to GPU primitives later.
