## Languages

* English: [README.md](README.md)
* Русский: [README.ru.md](README.ru.md)
* Українська: [README.uk.md](README.uk.md)
* Polski: [README.pl.md](README.pl.md)
* Deutsch: [README.de.md](README.de.md)
* Français: [README.fr.md](README.fr.md)
* Español: [README.es.md](README.es.md)
* Čeština: [README.cs.md](README.cs.md)
* 日本語: このファイル

# X-Ray 歴史的修復

**X-Ray / S.T.A.L.K.E.R.: Shadow of Chornobyl** の歴史的なソースコード snapshot を修復するための非公式ワークスペースです。

このブランチは現在、**build 1935 時代の X-Ray source tree** を対象としており、**Visual Studio 2022**、**CMake**、Win32 runtime testing 向けに修復されています。

> これは公式リリースではなく、オリジナルの game assets は含まれていません。
>
> 注: 2007年の英語版タイトルでは **S.T.A.L.K.E.R.: Shadow of Chernobyl** という表記が使われていました。この日本語 README では **Chornobyl** という表記を使用しています。

## 現在の修復ブランチ

* `restoration-1935` — X-Ray build 1935 時代の歴史的な source state を VS2022/CMake 向けに修復するブランチです。

## 現在の状態

このプロジェクトは現在、修復された Win32 Debug configuration で動作する build 状態に到達しています。

複数の主要な engine components が Visual Studio 2022 上で compile / link できるように修復されています。

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

Runtime は engine を起動し、scripts を読み込み、server/client startup を初期化し、ALife data を読み込み、level loading に入るところまで到達できます。

これはまだ実験的な修復です。Build は正常に compile できる場合がありますが、runtime execution では missing data、古い serialization assumptions、renderer issues、debug assertions によってまだエラーが発生する可能性があります。

## 現在判明している runtime issue

現在の runtime testing state は texture loading まで到達し、生成済み sky cubemap data が不足しているために失敗する場合があります。

```text
Can't find texture 'sky\sky_11_cube#small'
```

調査メモ:

* `#small` は texture loader の一般的な suffix ではありません。
* これは生成された DDS texture の名前として期待されているものです。
* `Environment.cpp` は environment sky cubemap 用に `#small` を付加します。
* Editor-side code は、base sky cubemap textures から小さい sky cubemaps を生成しているようです。
* 一部の runtime data packages には、たとえば `sky_11_cube.dds` のような base sky cubemap は含まれていますが、生成済みの `sky_11_cube#small.dds` は含まれていません。

考えられる修復方針:

1. original/editor pipeline を使って、不足している `#small` cubemap data を復元または再生成する。
2. 不足している environment sky cubemaps のために、範囲を限定した runtime generator を追加する。
3. さらなる runtime debugging のために、`sky_11_cube#small` から `sky_11_cube` への一時的な env-sky-only fallback を使用する。

長期的に望ましい方針は、広範な texture loading errors を隠すのではなく、original behavior を保ち、不足している generated data を明確に document することです。

## これまでに修復されたもの

このブランチには、以下の compatibility work が含まれています。

* 古い Visual Studio 2003 時代の projects からの CMake target recovery
* Visual Studio 2022 Win32 build compatibility
* legacy calling convention と linker compatibility
* DirectX 9 renderer targets
* OpenAL/EAX sound target recovery
* Lua / Luabind 0.7 compatibility
* Boost 1.33.1 compatibility
* 古い STL / iterator / loop-scope issues
* 古い X-Ray server object と ALife serialization/runtime startup issues
* server/client level loading までの debug runtime startup
* X-Ray `.xp*` archives を list するための read-only archive listing helper tool

## リポジトリ構成

この repository は source-focused です。

オリジナルの game assets は含まれていません。

local runtime data、build outputs、logs、proprietary game packages は commit しないでください。

無視することを推奨する local artifacts:

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

## 必要な外部依存関係

修復された build は、古い SDK/library dependencies がローカルに存在することを前提としています。

修復中に使用された local dependency layout の例:

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

既知の dependency set:

* C++ desktop workload を含む Visual Studio 2022
* CMake
* DirectX 9 SDK-era headers/libs。この source snapshot で使われていた 2004-era SDK が望ましいです。
* Boost 1.33.1
* Luabind 0.7
* historical source tree と互換性のある Lua
* Loki legacy headers
* OpenAL
* EAX SDK headers/libs
* Xiph OGG/Vorbis libraries

正確な dependency roots は、local CMake configuration で調整できます。

## Configure

Configure command の例:

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

local CMake option names が異なる場合は、root `CMakeLists.txt` と toolchain discovery logic を確認してください。

## Build

推奨される Debug build command:

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:minimal //clp:ErrorsOnly
```

より詳細な diagnostics が必要な場合:

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:normal //clp:ErrorsOnly
```

Individual targets は個別に build できます。例:

```bat
cmake --build build --config Debug --target xrGame -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R1 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R2 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

## Runtime setup

Build された binaries は、互換性のある historical X-Ray build 1935 runtime folder に配置する必要があります。

想定される runtime folder contents:

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

オリジナルの runtime assets は、この repository では提供されません。Runtime data は、自分が lawful に所有するコピー、アーカイブ、または研究資料から用意する必要があります。

## Runtime launch の例

修復テスト中に使用された command の例:

```bat
XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freeinput -start "server(all/single/alife) client(localhost)"
```

Visual Studio debugger launch の例:

```bat
devenv /debugexe .\XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freeinput -start "server(all/single/alife) client(localhost)"
```

Runtime testing flags:

* `-freecursor` は DirectInput mouse device のみを non-exclusive にします。
* `-freeinput` は windowed debugging 用に mouse と keyboard の DirectInput devices を non-exclusive にします。


## Archive inspection helper

このブランチには、X-Ray `.xp*` archive entries を list するための read-only helper tool が含まれています。

```bat
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

使用例:

```bat
build\bin\xrArchiveList.exe D:\Projects\Github\stalker-dream\gamedata.xp0 > xp0_entries.txt
```

その後、listing を検索できます。

```bat
rg -i "sky_11|sky_11_cube|#small" xp0_entries.txt
```

この tool は diagnostics のみを目的としています。Archive を extract、modify、repack することはありません。

## Development notes

この修復では、可能な限り original behavior を保持することを目指しています。

修復中に使用された一般的なルール:

* root cause が必要性を証明しない限り、gameplay/config/script/data を編集しない。
* external SDK/toolchain files を編集しない。
* missing engine behavior に対して fake stubs を追加しない。
* target-local CMake/source/link fixes を優先する。
* compatibility fixes は狭く保ち、document する。
* missing data problems を黙って隠さない。

この legacy CRLF-heavy source tree 向けの推奨 whitespace check:

```bat
git -c core.whitespace=blank-at-eol,blank-at-eof,space-before-tab,cr-at-eol diff --check
```

通常の `git diff --check` は、古い files に対して CRLF-related warnings を大量に出すことがあります。

## Releases

推奨 release tag format:

```text
v1935-vs2022-alpha-1
```

推奨 release title:

```text
X-Ray / S.T.A.L.K.E.R. Build 1935 VS2022 Restoration Alpha 1
```

Release archives にはオリジナルの game assets を含めないでください。

推奨 binary package contents:

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

Optional separate debug symbols package:

```text
xray-1935-vs2022-alpha1-symbols.zip
```

## Development assistance

この修復作業は **OpenAI Codex** の支援を受けて開発されました。

Codex は AI coding assistant として、以下の用途に使用されました。

* CMake target reconstruction
* legacy Visual Studio project migration
* compatibility patch drafting
* build error investigation
* runtime crash analysis
* source navigation と refactoring suggestions
* diagnostic tooling drafts

すべての変更は、修復プロセスの一部として手動で reviewed、built、tested、curated されています。

目的は engine を積極的に modernize することではなく、historical source behavior を保持しながら、modern Windows toolchain 上で build と runtime を使用可能にすることです。

## Legal / assets notice

この repository は source restoration、build scripts、documentation、compatibility research を目的としています。

オリジナルの game assets、proprietary data packages、runtime content は含まれていません。

自分が lawful に所有するコピー、アーカイブ、または研究資料に由来する legally obtained compatible runtime data package と一緒にのみ使用してください。

## Keywords

X-Ray Engine, X-Ray 1935, STALKER 1935, S.T.A.L.K.E.R. build 1935, Shadow of Chornobyl, Shadow of Chernobyl, SoC, OpenXRay, Visual Studio 2022, VS2022, CMake, DirectX 9, Win32, legacy game engine restoration, source restoration.
