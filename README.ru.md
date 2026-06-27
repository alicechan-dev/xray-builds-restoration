## Languages

* English: [README.md](README.md)
* Русский: этот файл
* Українська: [README.uk.md](README.uk.md)
* Polski: [README.pl.md](README.pl.md)
* Deutsch: [README.de.md](README.de.md)
* Français: [README.fr.md](README.fr.md)
* Español: [README.es.md](README.es.md)
* Čeština: [README.cs.md](README.cs.md)
* 日本語: [README.ja.md](README.ja.md)

# Историческая реставрация X-Ray

Неофициальное рабочее пространство для реставрации исторических snapshot’ов исходного кода **X-Ray / S.T.A.L.K.E.R.: Shadow of Chernobyl**.

Эта ветка сейчас сосредоточена на **дереве исходного кода X-Ray эпохи build 1935**, восстановленном для **Visual Studio 2022**, **CMake** и Win32 runtime testing.

> Это не официальный релиз и не содержит оригинальных игровых ассетов.

## Текущая реставрационная ветка

* `restoration-1935` — VS2022/CMake-реставрационная ветка для исторического состояния исходников X-Ray эпохи build 1935.

## Текущий статус

Проект сейчас достигает рабочего состояния build для восстановленной конфигурации Win32 Debug.

Несколько основных компонентов движка были восстановлены так, чтобы они компилировались и линковались в Visual Studio 2022:

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

Runtime может запустить движок, загрузить scripts, инициализировать server/client startup, загрузить ALife data и войти в level loading.

Это всё ещё экспериментальная реставрация. Build может успешно компилироваться, но runtime execution всё ещё может упираться в missing data, старые serialization assumptions, renderer issues или debug assertions.

## Примечание о совместимости environment sky cubemap

В исторических runtime data могут отсутствовать сгенерированные environment sky cubemaps с `#small`, например:

```text
sky\sky_11_cube#small
```

Editor-side tooling изначально генерировал эти уменьшенные DDS cubemaps из базовых sky cubemap textures.

Восстановленный runtime теперь имеет узкий compatibility fallback: если сгенерированная environment sky cubemap с `#small` отсутствует, используется соответствующая базовая sky cubemap, а в лог пишется сообщение, например:

```text
missing generated env sky cubemap 'sky\sky_11_cube#small', using base 'sky\sky_11_cube'
```

Этот fallback применяется только к environment sky cubemaps. Это не общий workaround для отсутствующих textures.

## Что уже восстановлено

Эта ветка содержит compatibility work для:

* восстановления CMake targets из старых проектов эпохи Visual Studio 2003;
* Visual Studio 2022 Win32 build compatibility;
* legacy calling convention и linker compatibility;
* DirectX 9 renderer targets;
* восстановления OpenAL/EAX sound target;
* Lua / Luabind 0.7 compatibility;
* Boost 1.33.1 compatibility;
* старых STL / iterator / loop-scope issues;
* старых X-Ray server object и ALife serialization/runtime startup issues;
* debug runtime startup через server/client level loading;
* read-only archive listing helper tool для архивов X-Ray `.xp*`.

## Структура репозитория

Репозиторий сосредоточен на исходном коде.

Оригинальные игровые ассеты не включены.

Не коммитьте локальные runtime data, build outputs, logs или proprietary game packages.

Рекомендуемые локальные артефакты для игнорирования:

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

## Необходимые внешние зависимости

Восстановленный build ожидает, что старые SDK/library dependencies будут доступны локально.

Пример локальной структуры зависимостей, использованной во время реставрации:

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

Известный набор зависимостей:

* Visual Studio 2022 с C++ desktop workload
* CMake
* DirectX 9 SDK-era headers/libs, желательно 2004-era SDK, использованный этим source snapshot.
* Boost 1.33.1
* Luabind 0.7
* Lua, совместимая с историческим source tree
* Loki legacy headers
* OpenAL
* EAX SDK headers/libs
* Xiph OGG/Vorbis libraries

Точные dependency roots можно настроить в вашей локальной CMake configuration.

## Configure

Пример configure command:

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

Если названия ваших локальных CMake options отличаются, проверьте корневой `CMakeLists.txt` и toolchain discovery logic.

## Build

Рекомендуемая команда Debug build:

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:minimal //clp:ErrorsOnly
```

Для более подробной диагностики:

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:normal //clp:ErrorsOnly
```

Отдельные targets можно build’ить отдельно, например:

```bat
cmake --build build --config Debug --target xrGame -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R1 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R2 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

## Runtime setup

Собранные binaries нужно поместить в совместимую историческую runtime-папку X-Ray build 1935.

Ожидаемое содержимое runtime-папки:

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

Оригинальные runtime assets не предоставляются этим репозиторием. Runtime data должны поступать из вашей собственной законной копии, архива или исследовательских материалов.

## Пример runtime launch

Пример команды, использованной во время реставрационного тестирования:

```bat
XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freeinput -start "server(all/single/alife) client(localhost)"
```

Пример запуска через Visual Studio debugger:

```bat
devenv /debugexe .\XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freeinput -start "server(all/single/alife) client(localhost)"
```

Runtime testing flags:

* `-freecursor` делает только DirectInput mouse device non-exclusive.
* `-freeinput` делает mouse и keyboard DirectInput devices non-exclusive для windowed debugging.


## Archive inspection helper

Эта ветка содержит read-only helper tool для вывода entries архивов X-Ray `.xp*`:

```bat
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

Пример использования:

```bat
build\bin\xrArchiveList.exe D:\Projects\Github\stalker-dream\gamedata.xp0 > xp0_entries.txt
```

Затем можно искать по listing:

```bat
rg -i "sky_11|sky_11_cube|#small" xp0_entries.txt
```

Этот tool предназначен только для diagnostics. Он не extract, не modify и не repack архивы.

## Development notes

Эта реставрация старается сохранять original behavior там, где это возможно.

Общие правила, использованные во время реставрации:

* избегать редактирования gameplay/config/script/data, если root cause не доказывает, что это требуется;
* избегать редактирования внешних SDK/toolchain files;
* избегать fake stubs для missing engine behavior;
* предпочитать target-local CMake/source/link fixes;
* держать compatibility fixes узкими и задокументированными;
* не скрывать молча missing data problems.

Рекомендуемая whitespace check для этого старого CRLF-heavy source tree:

```bat
git -c core.whitespace=blank-at-eol,blank-at-eof,space-before-tab,cr-at-eol diff --check
```

Обычный `git diff --check` может сообщать шумные CRLF-related warnings на старых файлах.

## Releases

Рекомендуемый формат release tag:

```text
v1935-vs2022-alpha-1
```

Рекомендуемый release title:

```text
X-Ray / S.T.A.L.K.E.R. Build 1935 VS2022 Restoration Alpha 1
```

Release archives не должны содержать оригинальные игровые ассеты.

Рекомендуемое содержимое binary package:

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

Опциональный отдельный debug symbols package:

```text
xray-1935-vs2022-alpha1-symbols.zip
```

## Development assistance

Эта реставрационная работа была выполнена при помощи **OpenAI Codex**.

Codex использовался как AI coding assistant для:

* реконструкции CMake targets;
* миграции legacy Visual Studio projects;
* черновиков compatibility patches;
* исследования build errors;
* анализа runtime crashes;
* source navigation и refactoring suggestions;
* черновиков diagnostic tooling.

Все изменения вручную reviewed, built, tested и curated как часть процесса реставрации.

Цель — не агрессивно modernize движок, а сохранить historical source behavior, одновременно сделав build и runtime usable на современной Windows toolchain.

## Legal / assets notice

Этот репозиторий предназначен для source restoration, build scripts, documentation и compatibility research.

Оригинальные игровые ассеты, proprietary data packages и runtime content не включены.

Используйте это только с legally obtained compatible runtime data package из вашей собственной законной копии, архива или исследовательских материалов.

## Keywords

X-Ray Engine, X-Ray 1935, STALKER 1935, S.T.A.L.K.E.R. build 1935, Shadow of Chernobyl, SoC, OpenXRay, Visual Studio 2022, VS2022, CMake, DirectX 9, Win32, legacy game engine restoration, source restoration.
