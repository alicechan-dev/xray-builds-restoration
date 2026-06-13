cmake_minimum_required(VERSION 3.24)

project(XRayBuild1935 LANGUAGES C CXX RC)

set(XRAY_OUTPUT_ROOT "${CMAKE_BINARY_DIR}" CACHE PATH "Root for generated build outputs")
set(XRAY_RUNTIME_DIR "${XRAY_OUTPUT_ROOT}/bin" CACHE PATH "Directory for executables and runtime DLLs")
set(XRAY_LIBRARY_DIR "${XRAY_OUTPUT_ROOT}/lib" CACHE PATH "Directory for static and import libraries")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${XRAY_LIBRARY_DIR}")
set(CMAKE_PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb")

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}")
  set(CMAKE_PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb")
endforeach()

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  # xrCore.vcproj sets ExceptionHandling="FALSE"; CMake's MSVC defaults add
  # /EHsc globally, so remove it before applying the target's explicit /EHs-.
  string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
  message(WARNING "xrCore is the original VC2003-era code and currently expects an MSVC-compatible compiler. MinGW/GCC is useful for CMake syntax checks but is not expected to compile this target without source compatibility work.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(WARNING "The original xrCore project targets Win32. A 32-bit generator/toolset is the safest first CMake path.")
endif()

set(DXSDK_DIR "$ENV{DXSDK_DIR}" CACHE PATH "Legacy DirectX SDK root, usually from the DXSDK_DIR environment variable")
set(DIRECTX_SDK_DIR "$ENV{DIRECTX_SDK_DIR}" CACHE PATH "Legacy DirectX SDK root, alternative cache variable name")
set(XRAY_DXSDK_ROOT "" CACHE PATH "Optional legacy DirectX 9 SDK root; needed if dxerr9.h/dxerr9.lib are not provided by the active toolchain")

set(XRAY_EFFECTIVE_DXSDK_ROOT "")
foreach(candidate IN ITEMS "${XRAY_DXSDK_ROOT}" "${DXSDK_DIR}" "${DIRECTX_SDK_DIR}")
  if(candidate)
    set(XRAY_EFFECTIVE_DXSDK_ROOT "${candidate}")
    break()
  endif()
endforeach()

set(XRCORE_SOURCES
  xrCore/FTimer.cpp
  xrCore/stdafx.cpp
  xrCore/xrCore.cpp
  xrCore/xrCore.rc
  xrCore/_compressed_normal.cpp
  xrCore/_math.cpp
  xrCore/_sphere.cpp
  xrCore/cpuid.cpp
  xrCore/_std_extensions.cpp
  xrCore/clsid.cpp
  xrCore/xr_trims.cpp
  xrCore/FileSystem.cpp
  xrCore/FileSystem_borland.cpp
  xrCore/FS.cpp
  xrCore/LocatorAPI.cpp
  xrCore/LocatorAPI_Notifications.cpp
  xrCore/log.cpp
  xrCore/Xr_ini.cpp
  xrCore/xrMemory.cpp
  xrCore/xrMemory_align.cpp
  xrCore/xrMemory_debug.cpp
  xrCore/xrMemory_POOL.cpp
  xrCore/xrMemory_pso_Copy.cpp
  xrCore/xrMemory_pso_Fill.cpp
  xrCore/xrMemory_pso_Fill32.cpp
  xrCore/xrMemory_subst_borland.cpp
  xrCore/xrMemory_subst_msvc.cpp
  xrCore/xrDebug.cpp
  xrCore/LzHuf.cpp
  xrCore/rt_compressor.cpp
  xrCore/rt_lzo.cpp
  xrCore/crc32.cpp
  xrCore/xrsharedmem.cpp
  xrCore/xrstring.cpp
  xrCore/xrSyncronize.cpp
)

add_library(xrCore SHARED ${XRCORE_SOURCES})

set_target_properties(xrCore PROPERTIES
  OUTPUT_NAME "xrCore"
  # xrCore.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
  VS_GLOBAL_CharacterSet "MultiByte"
  # VC2003-era STL compatibility: keep removed adapter types such as
  # std::binary_function available instead of compiling as modern C++.
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${XRAY_LIBRARY_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(xrCore PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

# xrCore.vcproj has no AdditionalIncludeDirectories. The project directory is
# still made public because legacy consumers include headers as <xrCore.h>.
target_include_directories(xrCore
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/xrCore"
)

if(XRAY_EFFECTIVE_DXSDK_ROOT)
  target_include_directories(xrCore PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Include" "${XRAY_EFFECTIVE_DXSDK_ROOT}/include")
  target_link_directories(xrCore PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib/x86" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib/x86")
endif()

# Keep compatibility headers after any real SDK include path so a legacy
# DirectX SDK dxerr9.h wins when the user provides one.
target_include_directories(xrCore PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc")

target_compile_definitions(xrCore
  PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    XRCORE_EXPORTS
    # Old MSVC STL headers used _FARQ in allocator signatures; modern MSVC no
    # longer defines it, and the historical project treated it as an empty tag.
    _FARQ=
    # VS2022 hides removed STL adapters such as std::binary_function unless
    # this legacy compatibility switch is enabled for old code.
    _HAS_AUTO_PTR_ETC=1
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xrCore
    PRIVATE
      /nologo
      /W4
      /GR
      /Gz
      /EHs-
      /EHa-
  )

  # Legacy VC2003 settings such as /G7, /Wp64, and per-file assembler listings
  # are intentionally not forced yet; several are obsolete on modern MSVC.
  target_compile_options(xrCore PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Gy /Oi->
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy /Zc:wchar_t->
  )

  target_link_options(xrCore PRIVATE
    /nologo
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrCore PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/xrCore/stdafx.h>")

target_link_libraries(xrCore PRIVATE
  winmm
)

if(XRAY_EFFECTIVE_DXSDK_ROOT)
  # xrDebug.cpp has a legacy #pragma comment(lib, "dxerr9.lib"). Keeping this
  # explicit helps CMake-based link diagnostics point at the SDK requirement.
  target_link_libraries(xrCore PRIVATE dxerr9)
elseif(MSVC)
  # Modern Windows SDKs do not ship the old DirectX SDK dxerr9 helper. The
  # local compatibility header lets xrDebug.cpp fall through to its existing
  # FormatMessage path, and this suppresses the matching legacy pragma lib.
  target_link_options(xrCore PRIVATE /NODEFAULTLIB:dxerr9.lib)
endif()
