## Languages

* English: [README.md](README.md)
* Русский: [README.ru.md](README.ru.md)
* Polski: [README.pl.md](README.pl.md)
* Deutsch: [README.de.md](README.de.md)
* Français: [README.fr.md](README.fr.md)
* Čeština: [README.cs.md](README.cs.md)
* Español: [README.es.md](README.es.md)
* 日本語: [README.ja.md](README.ja.md)

# Історична реставрація X-Ray

Неофіційний робочий простір для реставрації історичних snapshot’ів вихідного коду **X-Ray / S.T.A.L.K.E.R.: Shadow of Chornobyl**.

Ця гілка наразі зосереджена на **дереві вихідного коду X-Ray епохи build 1935**, відновленому для **Visual Studio 2022**, **CMake** та Win32 runtime testing.

> Це не офіційний реліз і не містить оригінальних ігрових асетів.
>
> Примітка: оригінальна англомовна назва гри 2007 року використовувала написання **S.T.A.L.K.E.R.: Shadow of Chernobyl**. У цьому українському README використовується написання **Chornobyl**.

## Поточна реставраційна гілка

* `restoration-1935` — VS2022/CMake реставраційна гілка для історичного стану вихідного коду X-Ray епохи build 1935.

## Поточний стан

Проєкт наразі досягає робочого стану build для відновленої конфігурації Win32 Debug.

Кілька основних компонентів рушія було відновлено так, щоб вони компілювалися та лінкувалися у Visual Studio 2022:

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

Runtime може запустити рушій, завантажити scripts, ініціалізувати server/client startup, завантажити ALife data та перейти до level loading.

Це все ще експериментальна реставрація. Build може успішно компілюватися, але під час runtime execution усе ще можливі помилки через missing data, старі serialization assumptions, renderer issues або debug assertions.

## Поточна відома runtime-проблема

Поточний стан runtime testing доходить до завантаження текстур і може завершитися помилкою через відсутні згенеровані sky cubemap data:

```text
Can't find texture 'sky\sky_11_cube#small'
```

Нотатки розслідування:

* `#small` не є загальним suffix для texture loader.
* Це очікувана назва згенерованої DDS texture.
* `Environment.cpp` додає `#small` для environment sky cubemap.
* Editor-side code, схоже, генерує ці менші sky cubemaps із базових sky cubemap textures.
* Деякі runtime data packages містять базову sky cubemap, наприклад `sky_11_cube.dds`, але не містять згенеровану `sky_11_cube#small.dds`.

Можливі шляхи реставрації:

1. відновити або заново згенерувати відсутні `#small` cubemap data за допомогою original/editor pipeline;
2. додати вузький runtime generator для відсутніх environment sky cubemaps;
3. використати тимчасовий env-sky-only fallback із `sky_11_cube#small` на `sky_11_cube` для подальшого runtime debugging.

Бажаний довгостроковий підхід — зберегти original behavior і документувати missing generated data, а не широко приховувати texture loading errors.

## Що вже було відновлено

Ця гілка містить compatibility work для:

* відновлення CMake targets зі старих проєктів епохи Visual Studio 2003;
* Visual Studio 2022 Win32 build compatibility;
* legacy calling convention та linker compatibility;
* DirectX 9 renderer targets;
* відновлення OpenAL/EAX sound target;
* Lua / Luabind 0.7 compatibility;
* Boost 1.33.1 compatibility;
* старих STL / iterator / loop-scope issues;
* старих X-Ray server object та ALife serialization/runtime startup issues;
* debug runtime startup через server/client level loading;
* read-only archive listing helper tool для архівів X-Ray `.xp*`.

## Структура репозиторію

Репозиторій зосереджений на вихідному коді.

Оригінальні ігрові асети не включені.

Не комітьте локальні runtime data, build outputs, logs або proprietary game packages.

Рекомендовані локальні артефакти для ігнорування:

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

## Необхідні зовнішні залежності

Відновлений build очікує, що старі SDK/library dependencies будуть доступні локально.

Приклад локальної структури залежностей, використаної під час реставрації:

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

Відомий набір залежностей:

* Visual Studio 2022 з C++ desktop workload
* CMake
* DirectX 9 SDK-era headers/libs, бажано 2004-era SDK, використаний цим source snapshot.
* Boost 1.33.1
* Luabind 0.7
* Lua, сумісна з історичним source tree
* Loki legacy headers
* OpenAL
* EAX SDK headers/libs
* Xiph OGG/Vorbis libraries

Точні dependency roots можна налаштувати у вашій локальній CMake configuration.

## Configure

Приклад команди configure:

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

Якщо назви ваших локальних CMake options відрізняються, перевірте кореневий `CMakeLists.txt` і toolchain discovery logic.

## Build

Рекомендована команда Debug build:

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:minimal //clp:ErrorsOnly
```

Для детальнішої діагностики:

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:normal //clp:ErrorsOnly
```

Окремі targets можна build окремо, наприклад:

```bat
cmake --build build --config Debug --target xrGame -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R1 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R2 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

## Runtime setup

Зібрані binaries слід розмістити у сумісній історичній runtime-папці X-Ray build 1935.

Очікуваний вміст runtime-папки:

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

Оригінальні runtime assets не надаються цим репозиторієм.

## Приклад runtime launch

Приклад команди, використаної під час реставраційного тестування:

```bat
XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freecursor -start "server(all/single/alife) client(localhost)"
```

Приклад запуску через Visual Studio debugger:

```bat
devenv /debugexe .\XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freecursor -start "server(all/single/alife) client(localhost)"
```

## Archive inspection helper

Ця гілка містить read-only helper tool для переліку entries архівів X-Ray `.xp*`:

```bat
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

Приклад використання:

```bat
build\bin\xrArchiveList.exe D:\Projects\Github\stalker-dream\gamedata.xp0 > xp0_entries.txt
```

Потім можна пошукати у listing:

```bat
rg -i "sky_11|sky_11_cube|#small" xp0_entries.txt
```

Цей tool призначений лише для diagnostics. Він не extract, не modify і не repack архіви.

## Development notes

Ця реставрація намагається зберігати original behavior там, де це можливо.

Загальні правила, використані під час реставрації:

* уникати редагування gameplay/config/script/data, якщо root cause не доводить, що це потрібно;
* уникати редагування зовнішніх SDK/toolchain files;
* уникати fake stubs для missing engine behavior;
* надавати перевагу target-local CMake/source/link fixes;
* тримати compatibility fixes вузькими та задокументованими;
* не приховувати мовчки missing data problems.

Рекомендована whitespace check для цього старого CRLF-heavy source tree:

```bat
git -c core.whitespace=blank-at-eol,blank-at-eof,space-before-tab,cr-at-eol diff --check
```

Звичайний `git diff --check` може повідомляти шумні CRLF-related warnings на старих файлах.

## Releases

Рекомендований формат release tag:

```text
v1935-vs2022-alpha-1
```

Рекомендований release title:

```text
X-Ray / S.T.A.L.K.E.R. Build 1935 VS2022 Restoration Alpha 1
```

Release archives не повинні містити оригінальні ігрові асети.

Рекомендований вміст binary package:

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

Опціональний окремий debug symbols package:

```text
xray-1935-vs2022-alpha1-symbols.zip
```

## Development assistance

Ця реставраційна робота була виконана за допомогою **OpenAI Codex**.

Codex використовувався як AI coding assistant для:

* реконструкції CMake targets;
* міграції legacy Visual Studio projects;
* чернеток compatibility patches;
* дослідження build errors;
* аналізу runtime crashes;
* source navigation та refactoring suggestions;
* чернеток diagnostic tooling.

Усі зміни вручну reviewed, built, tested і curated як частина процесу реставрації.

Мета — не агресивно modernize рушій, а зберегти historical source behavior, водночас зробивши build і runtime usable на сучасному Windows toolchain.

## Legal / assets notice

Цей репозиторій призначений для source restoration, build scripts, documentation та compatibility research.

Оригінальні ігрові асети, proprietary data packages і runtime content не включені.

Використовуйте це лише з legally obtained compatible runtime data package.

## Keywords

X-Ray Engine, X-Ray 1935, STALKER 1935, S.T.A.L.K.E.R. build 1935, Shadow of Chornobyl, Shadow of Chernobyl, SoC, OpenXRay, Visual Studio 2022, VS2022, CMake, DirectX 9, Win32, legacy game engine restoration, source restoration.
