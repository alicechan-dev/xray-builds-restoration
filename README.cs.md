## Languages

* English: [README.md](README.md)
* Русский: [README.ru.md](README.ru.md)
* Українська: [README.uk.md](README.uk.md)
* Polski: [README.pl.md](README.pl.md)
* Deutsch: [README.de.md](README.de.md)
* Français: [README.fr.md](README.fr.md)

# Historická restaurace X-Ray

Neoficiální pracovní prostor pro restauraci historických snapshotů zdrojového kódu **X-Ray / S.T.A.L.K.E.R.: Shadow of Chornobyl**.

Tato větev se aktuálně zaměřuje na **zdrojový strom X-Ray z éry buildu 1935**, obnovený pro **Visual Studio 2022**, **CMake** a Win32 runtime testování.

> Toto není oficiální vydání a neobsahuje původní herní assety.
>
> Poznámka: původní anglický název hry z roku 2007 používal zápis **S.T.A.L.K.E.R.: Shadow of Chernobyl**. V tomto českém README se používá zápis **Chornobyl**.

## Aktuální restaurační větev

* `restoration-1935` — restaurační větev VS2022/CMake pro historický stav zdrojů X-Ray z éry buildu 1935.

## Aktuální stav

Projekt aktuálně dosahuje funkčního stavu buildu pro obnovenou konfiguraci Win32 Debug.

Několik hlavních komponent enginu bylo obnoveno tak, aby šly kompilovat a linkovat pod Visual Studio 2022:

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

Runtime dokáže spustit engine, načíst skripty, inicializovat server/client startup, načíst ALife data a dojít k načítání levelu.

Stále jde o experimentální restauraci. Build se může úspěšně zkompilovat, ale při runtime spuštění se mohou stále objevit chyby kvůli chybějícím datům, starým předpokladům serializace, problémům rendereru nebo debug assertions.

## Aktuální známý runtime problém

Aktuální stav runtime testování dojde k načítání textur a může selhat kvůli chybějícím vygenerovaným sky cubemap datům:

```text
Can't find texture 'sky\sky_11_cube#small'
```

Poznámky z vyšetřování:

* `#small` není obecný suffix texture loaderu.
* Je to očekávaný název vygenerované DDS textury.
* `Environment.cpp` přidává `#small` pro environment sky cubemap.
* Editor-side kód podle všeho generuje tyto menší sky cubemapy ze základních sky cubemap textur.
* Některé runtime data packages obsahují základní sky cubemap, například `sky_11_cube.dds`, ale neobsahují vygenerovanou `sky_11_cube#small.dds`.

Možné cesty restaurace:

1. obnovit nebo znovu vygenerovat chybějící `#small` cubemap data pomocí původního/editor pipeline;
2. přidat úzce zaměřený runtime generátor pro chybějící environment sky cubemapy;
3. použít dočasný env-sky-only fallback z `sky_11_cube#small` na `sky_11_cube` pro další runtime debugging.

Preferovaný dlouhodobý přístup je zachovat původní chování a dokumentovat chybějící generated data, místo aby se široce skrývaly chyby načítání textur.

## Co už bylo obnoveno

Tato větev obsahuje compatibility work pro:

* obnovu CMake targetů ze starých projektů z éry Visual Studio 2003;
* kompatibilitu buildu Visual Studio 2022 Win32;
* kompatibilitu starých calling conventions a chování linkeru;
* DirectX 9 renderer targets;
* obnovu OpenAL/EAX sound targetu;
* kompatibilitu Lua / Luabind 0.7;
* kompatibilitu Boost 1.33.1;
* staré problémy STL / iterator / loop-scope;
* staré X-Ray server object a ALife serialization/runtime startup problémy;
* debug runtime startup přes server/client level loading;
* read-only helper tool pro vypisování archivů X-Ray `.xp*`.

## Struktura repozitáře

Repozitář se zaměřuje na zdrojový kód.

Původní herní assety nejsou zahrnuty.

Necommitujte lokální runtime data, build outputs, logy ani proprietary game packages.

Doporučené lokální artefakty k ignorování:

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

## Požadované externí závislosti

Obnovený build očekává, že staré SDK/library dependencies budou dostupné lokálně.

Příklad lokální struktury závislostí použité během restaurace:

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

Známá sada závislostí:

* Visual Studio 2022 s C++ desktop workload
* CMake
* DirectX 9 SDK-era headers/libs, ideálně SDK z roku 2004 používané tímto source snapshotem.
* Boost 1.33.1
* Luabind 0.7
* Lua kompatibilní s historickým zdrojovým stromem
* Loki legacy headers
* OpenAL
* EAX SDK headers/libs
* Xiph OGG/Vorbis libraries

Přesné cesty k závislostem lze upravit v lokální konfiguraci CMake.

## Konfigurace

Příklad konfiguračního příkazu:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32 ^
  -DDIRECTX_ROOT="D:/Projects/Toolchains/DirectX" ^
  -DBOOST_ROOT="D:/Projects/Toolchains/boost_1_33_1" ^
  -DLUABIND_ROOT="D:/Projects/Toolchains/luabind-0.7/luabind" ^
  -DLOKI_ROOT="D:/Projects/Toolchains/loki-legacy" ^
  -DOPENAL_ROOT="D:/Projects/Toolchains/OpenAL" ^
  -DEAX_ROOT="D:/Projects/Toolchains/EAX" ^
  -DXIPH_ROOT="D:/Projects/Toolchains/Xiph-msvc"
```

Pokud se názvy vašich lokálních CMake options liší, zkontrolujte kořenový `CMakeLists.txt` a toolchain discovery logic.

## Build

Doporučený příkaz pro Debug build:

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:minimal //clp:ErrorsOnly
```

Pro podrobnější diagnostiku:

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:normal //clp:ErrorsOnly
```

Jednotlivé targets lze buildit samostatně, například:

```bat
cmake --build build --config Debug --target xrGame -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R1 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R2 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

## Runtime setup

Zbuilděné binaries je třeba vložit do kompatibilní historické runtime složky X-Ray build 1935.

Očekávaný obsah runtime složky:

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

Původní runtime assets nejsou tímto repozitářem poskytovány.

## Příklad runtime spuštění

Příklad příkazu použitého během restauračního testování:

```bat
XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freecursor -start "server(all/single/alife) client(localhost)"
```

Příklad spuštění přes Visual Studio debugger:

```bat
devenv /debugexe .\XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freecursor -start "server(all/single/alife) client(localhost)"
```

## Archive inspection helper

Tato větev obsahuje read-only helper tool pro vypisování položek archivů X-Ray `.xp*`:

```bat
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

Příklad použití:

```bat
build\bin\xrArchiveList.exe D:\Projects\Github\stalker-dream\gamedata.xp0 > xp0_entries.txt
```

Poté lze listing prohledat:

```bat
rg -i "sky_11|sky_11_cube|#small" xp0_entries.txt
```

Tento tool je určen pouze pro diagnostiku. Neextrahuje, nemění ani nepřebaluje archivy.

## Development notes

Tato restaurace se snaží zachovat původní chování tam, kde je to možné.

Obecná pravidla používaná během restaurace:

* vyhnout se úpravám gameplay/config/script/data, pokud root cause neprokáže, že je to nutné;
* vyhnout se úpravám externích SDK/toolchain souborů;
* vyhnout se fake stubs pro chybějící chování enginu;
* preferovat target-local CMake/source/link fixes;
* držet compatibility fixes úzké a zdokumentované;
* neskrývat potichu problémy s chybějícími daty.

Doporučená whitespace check pro tento starý CRLF-heavy source tree:

```bat
git -c core.whitespace=blank-at-eol,blank-at-eof,space-before-tab,cr-at-eol diff --check
```

Obyčejný `git diff --check` může u starých souborů hlásit hlučné CRLF-related warnings.

## Releases

Doporučený formát release tagu:

```text
v1935-vs2022-alpha-1
```

Doporučený release title:

```text
X-Ray / S.T.A.L.K.E.R. Build 1935 VS2022 Restoration Alpha 1
```

Release archives by neměly obsahovat původní herní assety.

Doporučený obsah binary package:

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

Volitelný samostatný balíček debug symbols:

```text
xray-1935-vs2022-alpha1-symbols.zip
```

## Development assistance

Tato restaurační práce byla vyvíjena s pomocí **OpenAI Codex**.

Codex byl použit jako AI coding assistant pro:

* rekonstrukci CMake targets;
* migraci legacy Visual Studio projektů;
* návrhy compatibility patches;
* vyšetřování build errors;
* analýzu runtime crashes;
* navigaci ve zdrojích a refactoring suggestions;
* návrhy diagnostic tooling.

Všechny změny jsou ručně reviewed, built, tested a curated jako součást restauračního procesu.

Cílem není engine agresivně modernizovat, ale zachovat historické chování zdrojů a zároveň učinit build a runtime použitelnými na moderní Windows toolchain.

## Legal / assets notice

Tento repozitář je určen pro source restoration, build scripts, documentation a compatibility research.

Původní herní assety, proprietary data packages a runtime content nejsou zahrnuty.

Používejte to pouze s legally obtained compatible runtime data package.

## Keywords

X-Ray Engine, X-Ray 1935, STALKER 1935, S.T.A.L.K.E.R. build 1935, Shadow of Chornobyl, Shadow of Chernobyl, SoC, OpenXRay, Visual Studio 2022, VS2022, CMake, DirectX 9, Win32, legacy game engine restoration, source restoration.
