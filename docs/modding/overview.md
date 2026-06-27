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

Current modding documentation is preliminary. It records policy, runtime layout assumptions, archive format questions, and SDK/tool restoration plans. It does not provide a complete SDK and does not implement a new unpacker.

Future work may include SDK/tool restoration, archive unpackers, format converters, format documentation, validators, and tutorials.

## Documentation Index

* [Runtime Layout](runtime-layout.md)
* [Assets Policy](assets-policy.md)
* [Tools and SDK Status](../tools/status.md)
* [Archive Unpacker Plan](../tools/unpacker-plan.md)
* [SDK Restoration Plan](../tools/sdk-restoration-plan.md)
* [Archive Formats](../formats/archives.md)

## Preliminary Roadmap

* Document runtime layout.
* Inventory SDK/tools.
* Document archive formats.
* Build minimal archive list/extract tool.
* Write modding tutorials.
