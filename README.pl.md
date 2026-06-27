## Languages

* English: [README.md](README.md)
* Русский: [README.ru.md](README.ru.md)
* Українська: [README.uk.md](README.uk.md)
* Polski: ten plik
* Deutsch: [README.de.md](README.de.md)
* Français: [README.fr.md](README.fr.md)
* Español: [README.es.md](README.es.md)
* Čeština: [README.cs.md](README.cs.md)
* 日本語: [README.ja.md](README.ja.md)

# Historyczna restauracja X-Ray

Nieoficjalne środowisko robocze do restauracji historycznych snapshotów kodu źródłowego **X-Ray / S.T.A.L.K.E.R.: Shadow of Chornobyl**.

Ta gałąź skupia się obecnie na **drzewie źródłowym X-Ray z epoki builda 1935**, przywracanym pod **Visual Studio 2022**, **CMake** oraz testy runtime Win32.

> To nie jest oficjalne wydanie i nie zawiera oryginalnych assetów gry.
>
> Uwaga: oryginalny anglojęzyczny tytuł z 2007 roku używał pisowni **S.T.A.L.K.E.R.: Shadow of Chernobyl**. W tym polskim README używana jest pisownia **Chornobyl**.

## Bieżąca gałąź restauracyjna

* `restoration-1935` — gałąź restauracji VS2022/CMake dla historycznego stanu źródeł X-Ray z epoki builda 1935.

## Aktualny status

Projekt osiąga obecnie działający stan builda dla odtworzonej konfiguracji Win32 Debug.

Kilka głównych komponentów silnika zostało przywróconych tak, aby kompilowały się i linkowały pod Visual Studio 2022:

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

Runtime może uruchomić silnik, załadować skrypty, zainicjalizować server/client startup, załadować dane ALife i wejść w ładowanie poziomu.

To nadal eksperymentalna restauracja. Build może kompilować się poprawnie, ale podczas uruchamiania runtime nadal mogą pojawiać się błędy związane z brakującymi danymi, dawnymi założeniami serializacji, problemami renderera albo debug assertions.

## Uwaga o kompatybilności environment sky cubemap

Historyczne runtime data mogą nie zawierać wygenerowanych environment sky cubemaps z `#small`, na przykład:

```text
sky\sky_11_cube#small
```

Editor-side tooling pierwotnie generował te mniejsze DDS cubemapy z bazowych sky cubemap textures.

Odtworzony runtime ma teraz wąski compatibility fallback: jeśli wygenerowana environment sky cubemap z `#small` jest nieobecna, używana jest odpowiadająca jej bazowa sky cubemap, a do logu trafia komunikat, na przykład:

```text
missing generated env sky cubemap 'sky\sky_11_cube#small', using base 'sky\sky_11_cube'
```

Ten fallback dotyczy tylko environment sky cubemaps. Nie jest to ogólny workaround dla brakujących textures.

## Co zostało już odtworzone

Ta gałąź zawiera compatibility work dla:

* odtworzenia celów CMake ze starych projektów z epoki Visual Studio 2003;
* kompatybilności builda Visual Studio 2022 Win32;
* kompatybilności dawnych calling conventions oraz zachowania linkera;
* DirectX 9 renderer targets;
* odtworzenia OpenAL/EAX sound target;
* kompatybilności Lua / Luabind 0.7;
* kompatybilności Boost 1.33.1;
* dawnych problemów STL / iterator / loop-scope;
* dawnych problemów X-Ray server object oraz ALife serialization/runtime startup;
* debug runtime startup przez server/client level loading;
* read-only helper tool do listowania archiwów X-Ray `.xp*`.

## Struktura repozytorium

Repozytorium skupia się na kodzie źródłowym.

Oryginalne assety gry nie są dołączone.

Nie commituj lokalnych danych runtime, build outputs, logów ani proprietary game packages.

Zalecane lokalne artefakty do ignorowania:

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

## Wymagane zewnętrzne zależności

Odtworzony build oczekuje, że stare SDK/library dependencies będą dostępne lokalnie.

Przykładowy lokalny układ zależności używany podczas restauracji:

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

Znany zestaw zależności:

* Visual Studio 2022 z C++ desktop workload
* CMake
* DirectX 9 SDK-era headers/libs, najlepiej 2004-era SDK używane przez ten source snapshot.
* Boost 1.33.1
* Luabind 0.7
* Lua kompatybilne z historycznym drzewem źródeł
* Loki legacy headers
* OpenAL
* EAX SDK headers/libs
* Xiph OGG/Vorbis libraries

Dokładne dependency roots można dostosować w lokalnej konfiguracji CMake.

## Configure

Przykładowa komenda konfiguracji:

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

Jeśli nazwy lokalnych CMake options różnią się, sprawdź główny `CMakeLists.txt` oraz toolchain discovery logic.

## Build

Zalecana komenda builda Debug:

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:minimal //clp:ErrorsOnly
```

Dla bardziej szczegółowej diagnostyki:

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:normal //clp:ErrorsOnly
```

Poszczególne targets można budować osobno, na przykład:

```bat
cmake --build build --config Debug --target xrGame -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R1 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R2 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

## Runtime setup

Zbudowane binaries należy umieścić w kompatybilnym historycznym folderze runtime X-Ray build 1935.

Oczekiwana zawartość folderu runtime:

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

Oryginalne runtime assets nie są dostarczane przez to repozytorium. Runtime data muszą pochodzić z twojej własnej legalnej kopii, archiwum albo materiałów badawczych.

## Przykład uruchomienia runtime

Przykładowa komenda używana podczas testów restauracyjnych:

```bat
XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freeinput -start "server(all/single/alife) client(localhost)"
```

Przykład uruchomienia przez Visual Studio debugger:

```bat
devenv /debugexe .\XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freeinput -start "server(all/single/alife) client(localhost)"
```

Runtime testing flags:

* `-freecursor` sprawia, że tylko DirectInput mouse device jest non-exclusive.
* `-freeinput` sprawia, że mouse i keyboard DirectInput devices są non-exclusive dla windowed debugging.


## Archive inspection helper

Ta gałąź zawiera read-only helper tool do listowania wpisów archiwów X-Ray `.xp*`:

```bat
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

Przykład użycia:

```bat
build\bin\xrArchiveList.exe D:\Projects\Github\stalker-dream\gamedata.xp0 > xp0_entries.txt
```

Następnie można przeszukać listing:

```bat
rg -i "sky_11|sky_11_cube|#small" xp0_entries.txt
```

To narzędzie jest przeznaczone wyłącznie do diagnostyki. Nie wypakowuje, nie modyfikuje ani nie przepakowuje archiwów.

## Development notes

Ta restauracja stara się zachować oryginalne zachowanie tam, gdzie to możliwe.

Ogólne zasady używane podczas restauracji:

* unikać edytowania gameplay/config/script/data, chyba że root cause dowodzi, że jest to wymagane;
* unikać edytowania zewnętrznych plików SDK/toolchain;
* unikać fake stubs dla brakującego zachowania silnika;
* preferować target-local CMake/source/link fixes;
* utrzymywać compatibility fixes jako wąskie i udokumentowane;
* nie ukrywać po cichu problemów z brakującymi danymi.

Zalecany whitespace check dla tego starego CRLF-heavy source tree:

```bat
git -c core.whitespace=blank-at-eol,blank-at-eof,space-before-tab,cr-at-eol diff --check
```

Zwykłe `git diff --check` może zgłaszać szumne CRLF-related warnings na starych plikach.

## Releases

Zalecany format release tag:

```text
v1935-vs2022-alpha-1
```

Zalecany release title:

```text
X-Ray / S.T.A.L.K.E.R. Build 1935 VS2022 Restoration Alpha 1
```

Release archives nie powinny zawierać oryginalnych assetów gry.

Sugerowana zawartość binary package:

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

Opcjonalny osobny pakiet debug symbols:

```text
xray-1935-vs2022-alpha1-symbols.zip
```

## Development assistance

Ta praca restauracyjna została wykonana przy pomocy **OpenAI Codex**.

Codex był używany jako AI coding assistant do:

* rekonstrukcji CMake targets;
* migracji legacy Visual Studio projects;
* szkicowania compatibility patches;
* badania build errors;
* analizy runtime crashes;
* nawigacji po źródłach i sugestii refactoringu;
* szkiców diagnostic tooling.

Wszystkie zmiany są ręcznie reviewed, built, tested i curated jako część procesu restauracji.

Celem nie jest agresywna modernizacja silnika, lecz zachowanie historycznego zachowania źródeł przy jednoczesnym uczynieniu builda i runtime używalnymi na współczesnym Windows toolchain.

## Legal / assets notice

To repozytorium jest przeznaczone dla source restoration, build scripts, documentation oraz compatibility research.

Oryginalne assety gry, proprietary data packages oraz runtime content nie są dołączone.

Używaj tego wyłącznie z legally obtained compatible runtime data package pochodzącym z twojej własnej legalnej kopii, archiwum albo materiałów badawczych.

## Keywords

X-Ray Engine, X-Ray 1935, STALKER 1935, S.T.A.L.K.E.R. build 1935, Shadow of Chornobyl, Shadow of Chernobyl, SoC, OpenXRay, Visual Studio 2022, VS2022, CMake, DirectX 9, Win32, legacy game engine restoration, source restoration.
