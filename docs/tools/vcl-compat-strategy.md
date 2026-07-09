# ECore VCL Compatibility Strategy

## Scope

This document defines the boundary for restoring `Editors/ECore` with MSVC and
CMake. It does not propose a VCL GUI reimplementation and does not make the
experimental `xrECore` target buildable by itself.

The source audit covered `AnsiString`, `TShiftState`, `TMsgDlgType`,
`TModalResult`, `TForm`, `TFrame`, `TComponent`, `TObject`, `Application`,
`__fastcall`, `__closure`, `USEFORM`, ElPack includes, and `.dfm` resources.

## Audit Results

| Category | ECore evidence | Classification |
|---|---|---|
| `AnsiString` | Present in 42 files and 126 source occurrences. Used by object, thumbnail, image, sound, particle, export, logging, and UI code. | Non-GUI value type with Borland-specific semantics, but part of public/editor data APIs. |
| `TShiftState` | Present in 13 files. Uses `Contains`, `Clear`, `<<`, and `>>` with mouse and modifier values. | Input flag set; separable in principle, but currently embedded in UI interfaces. |
| `TMsgDlgType` and dialog values | Present in logging and many call sites. Includes severity values, button sets, and modal results. | Logging severity is separable; button/modal behavior belongs to a UI service boundary. |
| `TForm`, `TComponent`, `TObject`, `Application` | Used by forms, color selection, main UI, window state, and message pumping. | True VCL runtime/UI dependency; never provide placeholder behavior. |
| `__fastcall` | Used by callbacks and exported editor methods. MSVC accepts the calling convention. | Compiler ABI concern, not a VCL emulation target. |
| `__closure` | Used by the render callback type in `D3DUtils.h`. | Borland member-closure extension requiring a deliberate callback migration. |
| `USEFORM` and `.dfm` | Three form units: image editor, log form, and sound editor. | True Borland form/runtime dependency; remain outside the MSVC partial target. |
| `ElTree.hpp` | Included by material and form headers. | External ElPack UI dependency, separate from value-type compatibility. |

No existing C++ compatibility implementation for these VCL types was found in
the repository. Other editor generations repeat the Borland types rather than
providing a portable alternative. The small `TMsgDlgType` enum in the legacy
`gauss` tool is useful evidence for values, but it is not a complete ECore
dialog abstraction.

## Chosen Strategy

Use a staged, narrow form of Strategy A for non-GUI value semantics only:

1. Specify and test an isolated compatibility library before including it from
   ECore.
2. Limit its first candidate to the audited `AnsiString` surface.
3. Treat input flags and logging values as later, separate contracts.
4. Keep forms, components, application/message-pump behavior, Borland
   closures, `.dfm` resources, and ElPack outside the compatibility library.
5. Do not change the existing `xrECore` source list until the compatibility
   contracts and the non-VCL library boundary are proven.

Splitting directly into `xrECoreBase` and `xrECoreVcl` is premature because the
shared PCH and retained APIs currently expose VCL types across both Engine and
Editor directories. The compatibility contracts are needed before a source
split can be evaluated honestly.

## `AnsiString` Contract To Prove

The ECore audit found this required surface:

* construction and assignment from `LPCSTR`, other strings, and integral values;
* `c_str`;
* concatenation, append, comparison, and ordering;
* `LowerCase` and `UpperCase`;
* `IsEmpty` and one-based `Length`;
* `sprintf`-style formatting;
* compatibility with existing path helpers and containers.

Tests must capture Borland-compatible return values, indexing expectations,
temporary lifetime, formatting truncation behavior, and case conversion before
the type is used by `xrECore`. A simple `typedef std::string AnsiString` does
not satisfy this contract.

## Explicit Non-Goals

* No `TForm`, `TFrame`, `TComponent`, `TObject`, or `Application` placeholders.
* No fake dialog implementation.
* No ElPack widget substitutes.
* No `.dfm` conversion.
* No opportunistic source exclusions to obtain a nominally successful DLL.
* No compatibility definitions injected globally into runtime targets.

## Next Safe Implementation

Create an opt-in, standalone compatibility test target for the audited
`AnsiString` contract. Keep it disconnected from `xrECore` until those tests
pass. After that, replace only non-GUI string usage in a small ECore slice and
reassess whether an `xrECoreBase` target has a coherent API and dependency set.

Until then, `xrECore` remains an honest build probe blocked first by
`AnsiString` in `Editors/ECore/stdafx.h`.
