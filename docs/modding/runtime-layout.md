# Runtime Layout

This page is a preliminary reference for the expected runtime layout used by the restored historical X-Ray build 1935 engine/runtime.

The project currently focuses on source restoration. Runtime documentation is being added so future modders, tool authors, and compatibility testers can understand how the restored binaries are expected to relate to local data supplied by the user.

## Expected Root Files

A compatible local runtime folder typically contains engine binaries, configuration files, and local data supplied by the user:

```text
XR_3DA.exe
xrCore.dll
xrGame.dll
xrRender_R1.dll
xrRender_R2.dll
xrSound.dll
xrLUA.dll
xrXMLParser.dll
xrCDB.dll
xrParticles.dll
xrNetServer.dll
xrCPU_Pipe.dll
fsgame.ltx
user_*.ltx
gamedata.xp0
gamedata/
savedgames/
```

The exact set of DLLs and data paths can vary by build configuration and test scenario.

## Data Sources

Original game assets, archive files, extracted gamedata, repacks, cracks, and leaked runtime packages are not distributed by this repository.

Users are responsible for providing their own legally obtained compatible runtime data. Any future tools are intended for lawful research, compatibility testing, and use with the user's own legally obtained data.

## Future Documentation

Future updates may document:

* `fsgame.ltx` path resolution;
* archive and unpacked `gamedata/` precedence;
* expected DLL placement;
* editor and SDK output folders;
* save, log, and user configuration locations.
