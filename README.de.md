## Languages

* English: [README.md](README.md)
* Русский: [README.ru.md](README.ru.md)
* Українська: [README.uk.md](README.uk.md)
* Polski: [README.pl.md](README.pl.md)
* Deutsch: diese Datei
* Français: [README.fr.md](README.fr.md)
* Español: [README.es.md](README.es.md)
* Čeština: [README.cs.md](README.cs.md)
* 日本語: [README.ja.md](README.ja.md)

# Historische X-Ray-Restaurierung

Inoffizieller Arbeitsbereich zur Restaurierung historischer Source-Snapshots von **X-Ray / S.T.A.L.K.E.R.: Shadow of Chornobyl**.

Dieser Branch konzentriert sich derzeit auf den **X-Ray-Source-Tree aus der Build-1935-Ära**, restauriert für **Visual Studio 2022**, **CMake** und Win32-Runtime-Tests.

> Dies ist kein offizielles Release und enthält keine originalen Game Assets.
>
> Hinweis: Der ursprüngliche englischsprachige Titel von 2007 verwendete die Schreibweise **S.T.A.L.K.E.R.: Shadow of Chernobyl**. In diesem deutschen README wird die Schreibweise **Chornobyl** verwendet.

## Aktueller Restaurierungs-Branch

* `restoration-1935` — VS2022/CMake-Restaurierungsbranch für den historischen X-Ray-Source-Stand aus der Build-1935-Ära.

## Aktueller Status

Das Projekt erreicht derzeit einen funktionsfähigen Build-Zustand für die wiederhergestellte Win32-Debug-Konfiguration.

Mehrere wichtige Engine-Komponenten wurden so restauriert, dass sie unter Visual Studio 2022 kompilieren und linken:

* `xrCore`
* `xrCDB`
* `xrLUA`
* `xrXMLParser`
* `xrParticles`
* `xrCompress`
* `xrSE_Factory`
* `xrNetServer`
* `xrSound`
* `XR_3DA`
* `xrCPU_Pipe`
* `xrRender_R1`
* `xrRender_R2`
* `xrGame`

Die Runtime kann die Engine starten, Scripts laden, den Server/Client-Startup initialisieren, ALife-Daten laden und in das Level Loading eintreten.

Dies ist weiterhin eine experimentelle Restaurierung. Der Build kann erfolgreich kompilieren, aber die Runtime-Ausführung kann weiterhin auf fehlende Daten, alte Serialisierungsannahmen, Renderer-Probleme oder Debug Assertions stoßen.

## Kompatibilitätsnotiz zu Environment-Sky-Cubemaps

Historische Runtime data können generierte `#small` Environment-Sky-Cubemaps vermissen, zum Beispiel:

```text
sky\sky_11_cube#small
```

Editor-seitiges Tooling generierte diese kleineren DDS-Cubemaps ursprünglich aus den Basis-Sky-Cubemap-Texturen.

Die restaurierte Runtime besitzt jetzt einen eng begrenzten Compatibility Fallback: Wenn eine generierte `#small` Environment-Sky-Cubemap fehlt, verwendet sie die passende Basis-Sky-Cubemap und schreibt den Fallback ins Log, zum Beispiel:

```text
missing generated env sky cubemap 'sky\sky_11_cube#small', using base 'sky\sky_11_cube'
```

Dieser Fallback gilt nur für Environment-Sky-Cubemaps. Er ist kein allgemeiner Workaround für fehlende Texturen.

## Was bisher restauriert wurde

Dieser Branch enthält Compatibility Work für:

* Wiederherstellung von CMake Targets aus alten Projekten der Visual-Studio-2003-Ära;
* Visual-Studio-2022-Win32-Build-Kompatibilität;
* Kompatibilität mit alten Calling Conventions und Linker-Verhalten;
* DirectX-9-Renderer-Targets;
* Wiederherstellung des OpenAL/EAX-Sound-Targets;
* Lua-/Luabind-0.7-Kompatibilität;
* Boost-1.33.1-Kompatibilität;
* alte STL-/Iterator-/Loop-Scope-Probleme;
* alte X-Ray Server Object und ALife Serialization/Runtime Startup Issues;
* Debug Runtime Startup durch Server/Client Level Loading;
* ein read-only Archive Listing Helper Tool für X-Ray-`.xp*`-Archive.

## Repository-Struktur

Das Repository ist source-focused.

Originale Game Assets sind nicht enthalten.

Committe keine lokalen Runtime-Daten, Build Outputs, Logs oder proprietary game packages.

Empfohlene lokale Artefakte zum Ignorieren:

```text
build/
*.log
log*.txt
xp0_entries.txt
*.pdb
*.ilk
*.obj
*.exe
*.dll
*.lib
*.exp
*.map
gamedata/
gamedata.xp*
savedgames/
```

## Erforderliche externe Abhängigkeiten

Der restaurierte Build erwartet, dass alte SDK-/Library-Dependencies lokal verfügbar sind.

Beispielhafte lokale Dependency-Struktur, die während der Restaurierung verwendet wurde:

```text
D:/Projects/Toolchains/
  DirectX/
  boost_1_33_1/
  luabind-0.7/
    luabind/
  loki-legacy/
  OpenAL/
  EAX/
  Xiph-msvc/
```

Bekannter Abhängigkeitssatz:

* Visual Studio 2022 mit C++ Desktop Workload
* CMake
* DirectX-9-SDK-era headers/libs, vorzugsweise das 2004-era SDK, das von diesem Source Snapshot verwendet wurde.
* Boost 1.33.1
* Luabind 0.7
* Lua, kompatibel mit dem historischen Source Tree
* Loki legacy headers
* OpenAL
* EAX SDK headers/libs
* Xiph OGG/Vorbis libraries

Die genauen Dependency Roots können in deiner lokalen CMake-Konfiguration angepasst werden.

## Configure

Beispiel für den Configure-Befehl:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32 ^
  -DDIRECTX_ROOT="D:/Projects/Toolchains/DirectX/" ^
  -DBOOST_ROOT="D:/Projects/Toolchains/boost_1_33_1" ^
  -DLUABIND_ROOT="D:/Projects/Toolchains/luabind-0.7/luabind" ^
  -DLOKI_ROOT="D:/Projects/Toolchains/loki-legacy" ^
  -DOPENAL_ROOT="D:/Projects/Toolchains/OpenAL" ^
  -DEAX_ROOT="D:/Projects/Toolchains/EAX" ^
  -DXIPH_ROOT="D:/Projects/Toolchains/Xiph-msvc"
```

Wenn sich die Namen deiner lokalen CMake Options unterscheiden, prüfe das Root-`CMakeLists.txt` und die Toolchain Discovery Logic.

## Build

Empfohlener Debug-Build-Befehl:

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:minimal //clp:ErrorsOnly
```

Für detailliertere Diagnostik:

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:normal //clp:ErrorsOnly
```

Einzelne Targets können separat gebaut werden, zum Beispiel:

```bat
cmake --build build --config Debug --target xrGame -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R1 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R2 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

## Runtime setup

Die gebauten Binaries sollten in einen kompatiblen historischen X-Ray-Build-1935-Runtime-Ordner gelegt werden.

Erwarteter Inhalt des Runtime-Ordners:

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
user_koan.ltx
gamedata.xp0 or unpacked gamedata/
```

Originale Runtime Assets werden von diesem Repository nicht bereitgestellt. Runtime data müssen aus deiner eigenen rechtmäßigen Kopie, einem Archiv oder Forschungsmaterial stammen.

## Beispiel für Runtime-Start

Beispielbefehl, der während der Restaurierungstests verwendet wurde:

```bat
XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freeinput -start "server(all/single/alife) client(localhost)"
```

Beispiel für den Start über den Visual-Studio-Debugger:

```bat
devenv /debugexe .\XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freeinput -start "server(all/single/alife) client(localhost)"
```

Runtime testing flags:

* `-freecursor` macht nur das DirectInput mouse device non-exclusive.
* `-freeinput` macht mouse und keyboard DirectInput devices für windowed debugging non-exclusive.


## Archive inspection helper

Dieser Branch enthält ein read-only Helper Tool zum Auflisten von X-Ray-`.xp*`-Archiveinträgen:

```bat
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

Beispielverwendung:

```bat
build\bin\xrArchiveList.exe D:\Projects\Github\stalker-dream\gamedata.xp0 > xp0_entries.txt
```

Danach kann das Listing durchsucht werden:

```bat
rg -i "sky_11|sky_11_cube|#small" xp0_entries.txt
```

Dieses Tool ist ausschließlich für Diagnostik gedacht. Es extrahiert, verändert oder repackt keine Archive.

## Development notes

Diese Restaurierung versucht, das ursprüngliche Verhalten so weit wie möglich zu erhalten.

Allgemeine Regeln, die während der Restaurierung verwendet wurden:

* Gameplay/Config/Script/Data nicht bearbeiten, außer die Root Cause beweist, dass es erforderlich ist;
* externe SDK-/Toolchain-Dateien nicht bearbeiten;
* keine Fake Stubs für fehlendes Engine-Verhalten hinzufügen;
* target-local CMake/Source/Link Fixes bevorzugen;
* Compatibility Fixes eng begrenzen und dokumentieren;
* Probleme mit fehlenden Daten nicht stillschweigend verstecken.

Empfohlener Whitespace-Check für diesen alten CRLF-heavy Source Tree:

```bat
git -c core.whitespace=blank-at-eol,blank-at-eof,space-before-tab,cr-at-eol diff --check
```

Ein normales `git diff --check` kann bei alten Dateien viele CRLF-related Warnings melden.

## Releases

Empfohlenes Release-Tag-Format:

```text
v1935-vs2022-alpha-1
```

Empfohlener Release-Titel:

```text
X-Ray / S.T.A.L.K.E.R. Build 1935 VS2022 Restoration Alpha 1
```

Release-Archive sollten keine originalen Game Assets enthalten.

Vorgeschlagener Inhalt des Binary Package:

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
README_RUNTIME.txt
```

Optionales separates Debug-Symbols-Package:

```text
xray-1935-vs2022-alpha1-symbols.zip
```

## Development assistance

Diese Restaurierungsarbeit wurde mit Unterstützung von **OpenAI Codex** entwickelt.

Codex wurde als AI Coding Assistant verwendet für:

* Rekonstruktion von CMake Targets;
* Migration alter Visual-Studio-Projekte;
* Entwürfe für Compatibility Patches;
* Untersuchung von Build Errors;
* Analyse von Runtime Crashes;
* Source Navigation und Refactoring Suggestions;
* Entwürfe für Diagnostic Tooling.

Alle Änderungen werden im Rahmen des Restaurierungsprozesses manuell reviewed, built, tested und curated.

Das Ziel ist nicht, die Engine aggressiv zu modernisieren, sondern das historische Source-Verhalten zu bewahren und gleichzeitig Build und Runtime auf einer modernen Windows Toolchain nutzbar zu machen.

## Legal / assets notice

Dieses Repository ist für source restoration, build scripts, documentation und compatibility research gedacht.

Originale Game Assets, proprietary data packages und runtime content sind nicht enthalten.

Verwende dies nur mit einem legally obtained compatible runtime data package aus deiner eigenen rechtmäßigen Kopie, einem Archiv oder Forschungsmaterial.

## Keywords

X-Ray Engine, X-Ray 1935, STALKER 1935, S.T.A.L.K.E.R. build 1935, Shadow of Chornobyl, Shadow of Chernobyl, SoC, OpenXRay, Visual Studio 2022, VS2022, CMake, DirectX 9, Win32, legacy game engine restoration, source restoration.
