# Modding and Tools Overview

This repository currently focuses on source restoration for the historical X-Ray / S.T.A.L.K.E.R. build 1935 engine and runtime. Modding and SDK work is a future planning track intended to support lawful research, compatibility testing, and local experimentation with the user's own legally obtained game data.

The repository does not distribute original game assets, original game archives, extracted `gamedata/`, repacks, cracks, leaked runtime packages, leaked data, or gamedata dumps.

## Intended Workflow

A future modding workflow should look like this:

1. Obtain compatible game/runtime data separately from a lawful source.
2. Build the restored engine and any restored tools locally.
3. Use archive and SDK tools locally against the user's own data.
4. Inspect configs, scripts, formats, and assets for compatibility research.
5. Put changes in a separate mod directory rather than editing extracted base data in place.
6. Run the restored engine with a documented runtime layout and filesystem precedence rules.
7. Keep proprietary assets, original archives, extracted gamedata, and local runtime packages out of version control.

## Current Scope

Current modding documentation records policy, runtime layout assumptions, archive format research, and SDK/tool restoration plans. The tree now includes restored or scaffolded tool work such as `xrArchiveList` and `xr_unpack`, but it is not a complete SDK.

`xr_unpack` is intended for local lawful archives and uses explicit safety modes. Dry-run planning is recommended before any extraction, and extracted output must not be committed to this repository.

## SDK And Tool Restoration Track

Tool restoration should proceed from read-only inspection toward safe write-capable workflows:

* inventory historical tools and dependencies;
* restore read-only inspectors and validators first;
* add extraction or conversion only behind explicit flags and safety checks;
* defer large GUI editors and host plugins until formats and dependencies are documented;
* keep all proprietary runtime data outside version control.

The next practical tool targets are synthetic `xr_unpack` tests, optional archive filtering, and a read-only LTX/config validator scaffold.

## Documentation Index

* [Runtime Layout](runtime-layout.md)
* [Assets Policy](assets-policy.md)
* [Tools and SDK Status](../tools/status.md)
* [Archive Unpacker Plan](../tools/unpacker-plan.md)
* [SDK Restoration Plan](../tools/sdk-restoration-plan.md)
* [Archive Formats](../formats/archives.md)

## Preliminary Roadmap

* Keep runtime layout and filesystem precedence documented.
* Maintain the SDK/tools inventory.
* Expand archive format documentation only from proven source behavior.
* Harden `xr_unpack` with synthetic tests and safe filtering.
* Add read-only validators before converters/editors.
* Write modding tutorials that rely on lawful local data or synthetic fixtures.
