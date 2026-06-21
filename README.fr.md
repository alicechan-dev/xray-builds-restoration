## Languages

* English: [README.md](README.md)
* Русский: [README.ru.md](README.ru.md)
* Українська: [README.uk.md](README.uk.md)
* Polski: [README.pl.md](README.pl.md)
* Deutsch: [README.de.md](README.de.md)
* Čeština: [README.cs.md](README.cs.md)
* Español: [README.es.md](README.es.md)
* 日本語: [README.ja.md](README.ja.md)

# Restauration historique de X-Ray

Espace de travail non officiel pour la restauration de snapshots historiques du code source de **X-Ray / S.T.A.L.K.E.R.: Shadow of Chornobyl**.

Cette branche se concentre actuellement sur **l’arborescence source X-Ray de l’époque du build 1935**, restaurée pour **Visual Studio 2022**, **CMake** et les tests runtime Win32.

> Ceci n’est pas une version officielle et n’inclut pas les assets originaux du jeu.
>
> Note : le titre anglophone original de 2007 utilisait l’orthographe **S.T.A.L.K.E.R.: Shadow of Chernobyl**. Dans ce README français, l’orthographe **Chornobyl** est utilisée.

## Branche de restauration actuelle

* `restoration-1935` — branche de restauration VS2022/CMake pour l’état historique des sources X-Ray de l’époque du build 1935.

## État actuel

Le projet atteint actuellement un état de build fonctionnel pour la configuration Win32 Debug restaurée.

Plusieurs composants majeurs du moteur ont été restaurés afin de compiler et linker sous Visual Studio 2022 :

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

Le runtime peut démarrer le moteur, charger les scripts, initialiser le server/client startup, charger les données ALife et entrer dans le chargement du niveau.

Cela reste une restauration expérimentale. Le build peut compiler correctement, mais l’exécution runtime peut encore rencontrer des erreurs liées à des données manquantes, à d’anciennes hypothèses de sérialisation, à des problèmes de renderer ou à des debug assertions.

## Problème runtime actuel connu

L’état actuel des tests runtime atteint le chargement des textures et peut échouer à cause de données sky cubemap générées manquantes :

```text
Can't find texture 'sky\sky_11_cube#small'
```

Notes d’investigation :

* `#small` n’est pas un suffixe générique du texture loader.
* C’est le nom attendu d’une texture DDS générée.
* `Environment.cpp` ajoute `#small` pour l’environment sky cubemap.
* Le code editor-side semble générer ces sky cubemaps réduites à partir des textures sky cubemap de base.
* Certains runtime data packages contiennent la sky cubemap de base, par exemple `sky_11_cube.dds`, mais pas la texture générée `sky_11_cube#small.dds`.

Pistes possibles pour la restauration :

1. récupérer ou régénérer les données cubemap `#small` manquantes via le pipeline original/editor ;
2. ajouter un générateur runtime étroitement limité pour les environment sky cubemaps manquantes ;
3. utiliser un fallback temporaire env-sky-only de `sky_11_cube#small` vers `sky_11_cube` pour poursuivre le runtime debugging.

L’approche préférable à long terme est de préserver le comportement original et de documenter les generated data manquantes, plutôt que de masquer largement les erreurs de chargement de textures.

## Ce qui a déjà été restauré

Cette branche inclut du travail de compatibilité pour :

* la récupération des targets CMake à partir d’anciens projets de l’époque Visual Studio 2003 ;
* la compatibilité de build Visual Studio 2022 Win32 ;
* la compatibilité avec les anciennes calling conventions et le comportement du linker ;
* les targets renderer DirectX 9 ;
* la récupération du sound target OpenAL/EAX ;
* la compatibilité Lua / Luabind 0.7 ;
* la compatibilité Boost 1.33.1 ;
* d’anciens problèmes STL / iterator / loop-scope ;
* d’anciens problèmes X-Ray server object et ALife serialization/runtime startup ;
* le debug runtime startup via le chargement server/client du niveau ;
* un outil helper read-only pour lister les archives X-Ray `.xp*`.

## Structure du dépôt

Le dépôt est centré sur le code source.

Les assets originaux du jeu ne sont pas inclus.

Ne committez pas les données runtime locales, les build outputs, les logs ou les proprietary game packages.

Artefacts locaux recommandés à ignorer :

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

## Dépendances externes requises

Le build restauré attend que d’anciennes dépendances SDK/library soient disponibles localement.

Exemple de structure locale de dépendances utilisée pendant la restauration :

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

Ensemble de dépendances connu :

* Visual Studio 2022 avec le workload C++ desktop
* CMake
* headers/libs DirectX 9 SDK-era, de préférence le SDK de l’époque 2004 utilisé par ce source snapshot.
* Boost 1.33.1
* Luabind 0.7
* Lua compatible avec l’arborescence source historique
* Loki legacy headers
* OpenAL
* EAX SDK headers/libs
* bibliothèques Xiph OGG/Vorbis

Les chemins exacts vers les dependency roots peuvent être ajustés dans votre configuration CMake locale.

## Configure

Exemple de commande de configuration :

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

Si les noms de vos options CMake locales diffèrent, inspectez le `CMakeLists.txt` racine et la logique de toolchain discovery.

## Build

Commande de build Debug recommandée :

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:minimal //clp:ErrorsOnly
```

Pour un diagnostic plus détaillé :

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:normal //clp:ErrorsOnly
```

Les targets individuels peuvent être build séparément, par exemple :

```bat
cmake --build build --config Debug --target xrGame -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R1 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R2 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

## Runtime setup

Les binaries buildés doivent être placés dans un dossier runtime historique compatible avec X-Ray build 1935.

Contenu attendu du dossier runtime :

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

Les runtime assets originaux ne sont pas fournis par ce dépôt.

## Exemple de lancement runtime

Exemple de commande utilisée pendant les tests de restauration :

```bat
XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freecursor -start "server(all/single/alife) client(localhost)"
```

Exemple de lancement via le debugger Visual Studio :

```bat
devenv /debugexe .\XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freecursor -start "server(all/single/alife) client(localhost)"
```

## Archive inspection helper

Cette branche inclut un outil helper read-only pour lister les entrées des archives X-Ray `.xp*` :

```bat
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

Exemple d’utilisation :

```bat
build\bin\xrArchiveList.exe D:\Projects\Github\stalker-dream\gamedata.xp0 > xp0_entries.txt
```

Ensuite, recherchez dans le listing :

```bat
rg -i "sky_11|sky_11_cube|#small" xp0_entries.txt
```

Cet outil est destiné au diagnostic uniquement. Il n’extrait, ne modifie et ne repack aucune archive.

## Notes de développement

Cette restauration essaie de préserver le comportement original lorsque c’est possible.

Règles générales utilisées pendant la restauration :

* éviter d’éditer gameplay/config/script/data sauf si la root cause prouve que c’est nécessaire ;
* éviter d’éditer les fichiers externes SDK/toolchain ;
* éviter les fake stubs pour les comportements moteur manquants ;
* préférer les target-local CMake/source/link fixes ;
* garder les compatibility fixes étroits et documentés ;
* ne pas masquer silencieusement les problèmes de données manquantes.

Whitespace check recommandé pour cet ancien source tree très CRLF-heavy :

```bat
git -c core.whitespace=blank-at-eol,blank-at-eof,space-before-tab,cr-at-eol diff --check
```

Un simple `git diff --check` peut signaler des warnings CRLF-related bruyants sur les anciens fichiers.

## Releases

Format de release tag recommandé :

```text
v1935-vs2022-alpha-1
```

Titre de release recommandé :

```text
X-Ray / S.T.A.L.K.E.R. Build 1935 VS2022 Restoration Alpha 1
```

Les release archives ne doivent pas inclure les assets originaux du jeu.

Contenu suggéré du binary package :

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

Package debug symbols séparé optionnel :

```text
xray-1935-vs2022-alpha1-symbols.zip
```

## Assistance au développement

Ce travail de restauration a été développé avec l’aide de **OpenAI Codex**.

Codex a été utilisé comme AI coding assistant pour :

* la reconstruction des CMake targets ;
* la migration des anciens projets Visual Studio ;
* la rédaction de brouillons de compatibility patches ;
* l’investigation des build errors ;
* l’analyse des runtime crashes ;
* la navigation dans les sources et les suggestions de refactoring ;
* les brouillons de diagnostic tooling.

Toutes les modifications sont manually reviewed, built, tested et curated dans le cadre du processus de restauration.

L’objectif n’est pas de moderniser agressivement le moteur, mais de préserver le comportement historique des sources tout en rendant le build et le runtime utilisables sur une toolchain Windows moderne.

## Legal / assets notice

Ce dépôt est destiné à la source restoration, aux build scripts, à la documentation et à la compatibility research.

Les assets originaux du jeu, les proprietary data packages et le runtime content ne sont pas inclus.

Utilisez ceci uniquement avec un legally obtained compatible runtime data package.

## Keywords

X-Ray Engine, X-Ray 1935, STALKER 1935, S.T.A.L.K.E.R. build 1935, Shadow of Chornobyl, Shadow of Chernobyl, SoC, OpenXRay, Visual Studio 2022, VS2022, CMake, DirectX 9, Win32, legacy game engine restoration, source restoration.
