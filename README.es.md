## Languages

* English: [README.md](README.md)
* Русский: [README.ru.md](README.ru.md)
* Українська: [README.uk.md](README.uk.md)
* Polski: [README.pl.md](README.pl.md)
* Deutsch: [README.de.md](README.de.md)
* Français: [README.fr.md](README.fr.md)
* Español: este archivo
* Čeština: [README.cs.md](README.cs.md)
* 日本語: [README.ja.md](README.ja.md)

# Restauración histórica de X-Ray

Espacio de trabajo no oficial para la restauración de snapshots históricos del código fuente de **X-Ray / S.T.A.L.K.E.R.: Shadow of Chornobyl**.

Esta rama se centra actualmente en el **árbol de código fuente de X-Ray de la época del build 1935**, restaurado para **Visual Studio 2022**, **CMake** y pruebas runtime Win32.

> Esta no es una versión oficial y no incluye los assets originales del juego.
>
> Nota: el título original en inglés de 2007 usaba la grafía **S.T.A.L.K.E.R.: Shadow of Chernobyl**. En este README en español se usa la grafía **Chornobyl**.

## Rama de restauración actual

* `restoration-1935` — rama de restauración VS2022/CMake para el estado histórico del código fuente de X-Ray de la época del build 1935.

## Estado actual

El proyecto actualmente alcanza un estado de build funcional para la configuración Win32 Debug restaurada.

Varios componentes principales del motor han sido restaurados para compilar y linkear bajo Visual Studio 2022:

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

El runtime puede iniciar el motor, cargar scripts, inicializar el server/client startup, cargar datos ALife y entrar en el proceso de carga del nivel.

Esto sigue siendo una restauración experimental. El build puede compilar correctamente, pero la ejecución runtime todavía puede encontrar datos faltantes, antiguas suposiciones de serialización, problemas del renderer o debug assertions.

## Nota de compatibilidad de environment sky cubemap

Los runtime data históricos pueden carecer de environment sky cubemaps generadas con `#small`, por ejemplo:

```text
sky\sky_11_cube#small
```

El tooling editor-side generaba originalmente estas DDS cubemaps más pequeñas a partir de las texturas sky cubemap base.

El runtime restaurado ahora tiene un compatibility fallback estrecho: si falta una environment sky cubemap generada con `#small`, usa la sky cubemap base correspondiente y registra el fallback en el log, por ejemplo:

```text
missing generated env sky cubemap 'sky\sky_11_cube#small', using base 'sky\sky_11_cube'
```

Este fallback solo se aplica a environment sky cubemaps. No es un workaround general para texturas faltantes.

## Qué se ha restaurado hasta ahora

Esta rama incluye trabajo de compatibilidad para:

* recuperación de targets CMake a partir de proyectos antiguos de la época de Visual Studio 2003;
* compatibilidad de build Visual Studio 2022 Win32;
* compatibilidad con antiguas calling conventions y comportamiento del linker;
* targets renderer DirectX 9;
* recuperación del sound target OpenAL/EAX;
* compatibilidad Lua / Luabind 0.7;
* compatibilidad Boost 1.33.1;
* antiguos problemas STL / iterator / loop-scope;
* antiguos problemas de X-Ray server object y ALife serialization/runtime startup;
* debug runtime startup a través de la carga server/client del nivel;
* una herramienta helper read-only para listar archivos X-Ray `.xp*`.

## Estructura del repositorio

El repositorio está centrado en el código fuente.

Los assets originales del juego no están incluidos.

No commitees datos runtime locales, build outputs, logs ni proprietary game packages.

Artefactos locales recomendados para ignorar:

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

## Dependencias externas requeridas

El build restaurado espera que las antiguas dependencias SDK/library estén disponibles localmente.

Ejemplo de estructura local de dependencias usada durante la restauración:

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

Conjunto conocido de dependencias:

* Visual Studio 2022 con C++ desktop workload
* CMake
* DirectX 9 SDK-era headers/libs, preferiblemente el SDK de 2004 usado por este source snapshot.
* Boost 1.33.1
* Luabind 0.7
* Lua compatible con el árbol histórico de código fuente
* Loki legacy headers
* OpenAL
* EAX SDK headers/libs
* Xiph OGG/Vorbis libraries

Las rutas exactas de las dependencias pueden ajustarse en tu configuración local de CMake.

## Configuración

Ejemplo de comando de configuración:

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

Si los nombres de tus opciones locales de CMake difieren, revisa el `CMakeLists.txt` raíz y la lógica de toolchain discovery.

## Build

Comando recomendado para Debug build:

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:minimal //clp:ErrorsOnly
```

Para diagnósticos más detallados:

```bat
cmake --build build --config Debug --target XR_3DA -- //m:1 //v:normal //clp:ErrorsOnly
```

Los targets individuales pueden buildearse por separado, por ejemplo:

```bat
cmake --build build --config Debug --target xrGame -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R1 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrRender_R2 -- //m:1 //v:normal //clp:ErrorsOnly
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

## Runtime setup

Los binaries buildeados deben colocarse en una carpeta runtime histórica compatible con X-Ray build 1935.

Contenido esperado de la carpeta runtime:

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

Los runtime assets originales no son proporcionados por este repositorio. Los runtime data deben proceder de tu propia copia legal, archivo o materiales de investigación.

## Ejemplo de lanzamiento runtime

Ejemplo de comando usado durante las pruebas de restauración:

```bat
XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freeinput -start "server(all/single/alife) client(localhost)"
```

Ejemplo de lanzamiento con el debugger de Visual Studio:

```bat
devenv /debugexe .\XR_3DA.exe -nocache -ltx user_koan.ltx -external -windowed -freeinput -start "server(all/single/alife) client(localhost)"
```

Runtime testing flags:

* `-freecursor` hace non-exclusive solo el DirectInput mouse device.
* `-freeinput` hace non-exclusive los DirectInput devices de mouse y keyboard para windowed debugging.


## Archive inspection helper

Esta rama incluye una herramienta helper read-only para listar entradas de archivos X-Ray `.xp*`:

```bat
cmake --build build --config Debug --target xrArchiveList -- //m:1 //v:minimal //clp:ErrorsOnly
```

Ejemplo de uso:

```bat
build\bin\xrArchiveList.exe D:\Projects\Github\stalker-dream\gamedata.xp0 > xp0_entries.txt
```

Después, busca en el listado:

```bat
rg -i "sky_11|sky_11_cube|#small" xp0_entries.txt
```

Esta herramienta está destinada únicamente a diagnósticos. No extrae, modifica ni repackea archivos.

## Notas de desarrollo

Esta restauración intenta preservar el comportamiento original siempre que sea posible.

Reglas generales usadas durante la restauración:

* evitar editar gameplay/config/script/data salvo que la root cause demuestre que es necesario;
* evitar editar archivos externos de SDK/toolchain;
* evitar fake stubs para comportamiento faltante del motor;
* preferir target-local CMake/source/link fixes;
* mantener los compatibility fixes estrechos y documentados;
* no ocultar silenciosamente problemas de datos faltantes.

Whitespace check recomendado para este antiguo source tree con mucho CRLF:

```bat
git -c core.whitespace=blank-at-eol,blank-at-eof,space-before-tab,cr-at-eol diff --check
```

Un `git diff --check` normal puede reportar warnings ruidosos relacionados con CRLF en archivos antiguos.

## Releases

Formato recomendado para release tag:

```text
v1935-vs2022-alpha-1
```

Título recomendado para release:

```text
X-Ray / S.T.A.L.K.E.R. Build 1935 VS2022 Restoration Alpha 1
```

Los release archives no deben incluir los assets originales del juego.

Contenido sugerido del binary package:

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

Paquete separado opcional de debug symbols:

```text
xray-1935-vs2022-alpha1-symbols.zip
```

## Asistencia de desarrollo

Este trabajo de restauración fue desarrollado con ayuda de **OpenAI Codex**.

Codex fue usado como AI coding assistant para:

* reconstrucción de CMake targets;
* migración de proyectos legacy de Visual Studio;
* borradores de compatibility patches;
* investigación de build errors;
* análisis de runtime crashes;
* navegación por el código fuente y sugerencias de refactoring;
* borradores de diagnostic tooling.

Todos los cambios son manually reviewed, built, tested y curated como parte del proceso de restauración.

El objetivo no es modernizar agresivamente el motor, sino preservar el comportamiento histórico del código fuente mientras se hace que el build y el runtime sean utilizables en una toolchain moderna de Windows.

## Legal / assets notice

Este repositorio está destinado a source restoration, build scripts, documentation y compatibility research.

Los assets originales del juego, proprietary data packages y runtime content no están incluidos.

Usa esto únicamente con un legally obtained compatible runtime data package procedente de tu propia copia legal, archivo o materiales de investigación.

## Keywords

X-Ray Engine, X-Ray 1935, STALKER 1935, S.T.A.L.K.E.R. build 1935, Shadow of Chornobyl, Shadow of Chernobyl, SoC, OpenXRay, Visual Studio 2022, VS2022, CMake, DirectX 9, Win32, legacy game engine restoration, source restoration.
