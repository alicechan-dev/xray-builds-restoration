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
set(D3DXSDK_DIR "$ENV{D3DXSDK_DIR}" CACHE PATH "Legacy DirectX SDK root, alternative historical cache variable name")
set(DIRECTX_SDK_DIR "$ENV{DIRECTX_SDK_DIR}" CACHE PATH "Legacy DirectX SDK root, alternative cache variable name")
set(XRAY_DXSDK_ROOT "" CACHE PATH "Optional legacy DirectX 9 SDK root; needed for legacy headers such as dplay8.h")

set(XRAY_EFFECTIVE_DXSDK_ROOT "")
foreach(candidate IN ITEMS "${XRAY_DXSDK_ROOT}" "${D3DXSDK_DIR}" "${DXSDK_DIR}" "${DIRECTX_SDK_DIR}")
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
  xrCore/_quaternion.cpp
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

# Keep xrCore on the local dxerr9.h fallback with modern MSVC even when a
# legacy DirectX SDK is configured for other targets such as xrNetServer. The
# old dxerr9.lib imports _vsnwprintf and is not link-compatible with VS2022.
target_include_directories(xrCore PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc")

if(XRAY_EFFECTIVE_DXSDK_ROOT)
  target_include_directories(xrCore PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Include" "${XRAY_EFFECTIVE_DXSDK_ROOT}/include")
  target_link_directories(xrCore PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib/x86" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib/x86")
endif()

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

if(MSVC)
  # The local compatibility header lets xrDebug.cpp fall through to its existing
  # FormatMessage path, and this suppresses the matching legacy pragma lib even
  # when XRAY_DXSDK_ROOT is set for other DirectX headers.
  target_link_options(xrCore PRIVATE /NODEFAULTLIB:dxerr9.lib)
endif()

# First modularization phase: tools own their local CMake target definitions.
# Runtime targets below remain in this root file until each can be moved with
# focused, target-by-target validation.
add_subdirectory(tools)

# Runtime target modularization is incremental; keep moving small targets one
# at a time while preserving target names and historical compatibility flags.
add_subdirectory(xr_3da/xrCDB)

set(XRLUA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xrLUA")
set(XRLUA_LEGACY_INCLUDE_DIR "" CACHE PATH "Optional path to the original xrLUA public Lua headers when xrLUA/include is absent")

if(NOT EXISTS "${XRLUA_DIR}/include/lua.h" AND NOT XRLUA_LEGACY_INCLUDE_DIR)
  message(WARNING "xrLUA.vcproj references xrLUA/include, but lua.h was not found there. Set XRLUA_LEGACY_INCLUDE_DIR to the original Lua public header directory to build xrLUA without source changes.")
endif()

set(XRLUA_SOURCES
  "${XRLUA_DIR}/src/lapi.c"
  "${XRLUA_DIR}/src/lcode.c"
  "${XRLUA_DIR}/src/ldebug.c"
  "${XRLUA_DIR}/src/ldo.c"
  "${XRLUA_DIR}/src/ldump.c"
  "${XRLUA_DIR}/src/lfunc.c"
  "${XRLUA_DIR}/src/lgc.c"
  "${XRLUA_DIR}/src/llex.c"
  "${XRLUA_DIR}/src/lmem.c"
  "${XRLUA_DIR}/src/lobject.c"
  "${XRLUA_DIR}/src/lopcodes.c"
  "${XRLUA_DIR}/src/lparser.c"
  "${XRLUA_DIR}/src/lstate.c"
  "${XRLUA_DIR}/src/lstring.c"
  "${XRLUA_DIR}/src/ltable.c"
  "${XRLUA_DIR}/src/ltests.c"
  "${XRLUA_DIR}/src/ltm.c"
  "${XRLUA_DIR}/src/lundump.c"
  "${XRLUA_DIR}/src/lvm.c"
  "${XRLUA_DIR}/src/lzio.c"
  "${XRLUA_DIR}/src/lib/lauxlib.c"
  "${XRLUA_DIR}/src/lib/lbaselib.c"
  "${XRLUA_DIR}/src/lib/ldblib.c"
  "${XRLUA_DIR}/src/lib/liolib.c"
  "${XRLUA_DIR}/src/lib/lmathlib.c"
  "${XRLUA_DIR}/src/lib/loadlib.c"
  "${XRLUA_DIR}/src/lib/lstrlib.c"
  "${XRLUA_DIR}/src/lib/ltablib.c"
  "${XRLUA_DIR}/src/xr_LuaAllocator.c"
  "${XRLUA_DIR}/src/xr_print.c"
  "${XRLUA_DIR}/StdAfx.cpp"
)

set(XRLUA_CXX_SOURCES ${XRLUA_SOURCES})
list(REMOVE_ITEM XRLUA_CXX_SOURCES "${XRLUA_DIR}/src/xr_LuaAllocator.c")

add_library(xrLUA SHARED ${XRLUA_SOURCES})

set_source_files_properties(${XRLUA_CXX_SOURCES} PROPERTIES LANGUAGE CXX)
set_source_files_properties("${XRLUA_DIR}/src/xr_LuaAllocator.c" PROPERTIES LANGUAGE C)

set_target_properties(xrLUA PROPERTIES
  OUTPUT_NAME "xrLUA"
  # xrLUA.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${XRAY_LIBRARY_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(xrLUA PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrLUA
  PUBLIC
    "${XRLUA_DIR}/include"
  PRIVATE
    "${XRLUA_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc"
)

if(XRLUA_LEGACY_INCLUDE_DIR)
  target_include_directories(xrLUA PUBLIC "${XRLUA_LEGACY_INCLUDE_DIR}")
endif()

target_compile_definitions(xrLUA
  PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    XRLUA_EXPORTS
    # xrLUA includes xrCore.h, so it needs the same modern MSVC compatibility
    # switches for legacy xrCore/STL headers even though they were implicit in
    # the original VC2003 environment.
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xrLUA
    PRIVATE
      /nologo
      /W3
      /GR
      /Gz
      /EHs-
      /EHa-
  )

  # Mirrors xrLUA.vcproj optimization/runtime choices where modern MSVC still
  # supports them; obsolete VC2003 settings such as /G7 and /Wp64 are omitted.
  target_compile_options(xrLUA PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Oi->
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy /Zc:wchar_t->
  )

  target_link_options(xrLUA PRIVATE
    /nologo
    # stdafx.h has the original #pragma comment(lib, "x:\\xrCore.lib").
    # CMake links the xrCore target directly, so ignore only that hardcoded
    # X: drive default library directive.
    /NODEFAULTLIB:x:\\xrCore.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrLUA PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XRLUA_DIR}/stdafx.h>")

target_link_libraries(xrLUA PRIVATE
  xrCore
)

# Runtime target modularization is incremental; xrXMLParser is the first
# low-risk runtime component moved to a local CMakeLists.txt.
add_subdirectory(xrXMLParser)

set(XRCOMPRESS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xrCompress")

set(XRCOMPRESS_SOURCES
  "${XRCOMPRESS_DIR}/StdAfx.cpp"
  "${XRCOMPRESS_DIR}/xrCompress.cpp"
  "${XRCOMPRESS_DIR}/lzo/alloc.c"
  "${XRCOMPRESS_DIR}/lzo/io.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1_99.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1a.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1a_99.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1b_1.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1b_2.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1b_3.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1b_4.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1b_5.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1b_6.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1b_7.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1b_8.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1b_9.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1b_99.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1b_9x.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1b_cc.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1b_d1.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1b_d2.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1b_rr.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1b_xx.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1c_1.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1c_2.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1c_3.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1c_4.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1c_5.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1c_6.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1c_7.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1c_8.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1c_9.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1c_99.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1c_9x.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1c_cc.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1c_d1.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1c_d2.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1c_rr.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1c_xx.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1f_1.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1f_9x.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1f_d1.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1f_d2.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1x_1.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1x_1k.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1x_1l.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1x_1o.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1x_9x.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1x_d1.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1x_d2.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1x_d3.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1x_o.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1y_1.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1y_9x.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1y_d1.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1y_d2.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1y_d3.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1y_o.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1z_9x.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1z_d1.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1z_d2.c"
  "${XRCOMPRESS_DIR}/lzo/lzo1z_d3.c"
  "${XRCOMPRESS_DIR}/lzo/lzo2a_9x.c"
  "${XRCOMPRESS_DIR}/lzo/lzo2a_d1.c"
  "${XRCOMPRESS_DIR}/lzo/lzo2a_d2.c"
  "${XRCOMPRESS_DIR}/lzo/lzo_crc.c"
  "${XRCOMPRESS_DIR}/lzo/lzo_dll.c"
  "${XRCOMPRESS_DIR}/lzo/lzo_init.c"
  "${XRCOMPRESS_DIR}/lzo/lzo_ptr.c"
  "${XRCOMPRESS_DIR}/lzo/lzo_str.c"
  "${XRCOMPRESS_DIR}/lzo/lzo_util.c"
)

add_executable(xrCompress ${XRCOMPRESS_SOURCES})

set_target_properties(xrCompress PROPERTIES
  OUTPUT_NAME "xrCompress"
  # xrCompress.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(xrCompress PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrCompress
  PRIVATE
    "${XRCOMPRESS_DIR}"
    "${XRCOMPRESS_DIR}/lzo"
    "${CMAKE_CURRENT_SOURCE_DIR}/xrCore"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc"
)

target_compile_definitions(xrCompress
  PRIVATE
    WIN32
    _CONSOLE
    # xrCompress includes xrCore.h, so it needs the same modern MSVC
    # compatibility switches for legacy xrCore/STL headers even though they
    # were implicit in the original VC2003 environment.
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
)

if(MSVC)
  target_compile_options(xrCompress
    PRIVATE
      /nologo
      /W3
      /Gz
      $<$<COMPILE_LANGUAGE:CXX>:/GR /EHs- /EHa->
  )

  # Mirrors xrCompress.vcproj optimization/runtime choices where modern MSVC
  # still supports them; obsolete VC2003 settings such as /G7 are omitted.
  target_compile_options(xrCompress PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Gy>
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy>
  )

  target_link_options(xrCompress PRIVATE
    /nologo
    /SUBSYSTEM:CONSOLE
    # xrCompress.cpp has the original #pragma comment(lib, "x:\\xrCore.lib").
    # CMake links xrCore directly, so ignore only that hardcoded X: drive
    # default library directive.
    /NODEFAULTLIB:x:\\xrCore.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrCompress PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XRCOMPRESS_DIR}/StdAfx.h>")

target_link_libraries(xrCompress PRIVATE
  xrCore
  winmm
  odbc32
  odbccp32
)

add_subdirectory(xrParticles)

set(XRSE_FACTORY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xrSE_Factory")
set(XRSE_FACTORY_LUABIND_INCLUDE_DIR "" CACHE PATH "Optional path containing the legacy luabind public header tree used by xrSE_Factory")
set(XRAY_LOKI_ROOT "" CACHE PATH "Optional Loki include root containing typelist.h for smart_cast")
set(XRAY_BOOST_ROOT "" CACHE PATH "Optional Boost 1.30.x-compatible include root for legacy luabind consumers")

if(NOT XRAY_LOKI_ROOT)
  message(WARNING "xrSE_Factory smart_cast.h includes <typelist.h>, but typelist.h was not found in this snapshot. Set XRAY_LOKI_ROOT to the original Loki include root.")
elseif(NOT EXISTS "${XRAY_LOKI_ROOT}/typelist.h")
  message(WARNING "XRAY_LOKI_ROOT='${XRAY_LOKI_ROOT}' does not contain typelist.h.")
endif()

if(NOT XRSE_FACTORY_LUABIND_INCLUDE_DIR)
  message(WARNING "xrSE_Factory includes <luabind/...> headers, but this snapshot does not contain the matching luabind public header tree under xrSE_Factory. Set XRSE_FACTORY_LUABIND_INCLUDE_DIR to the original legacy luabind include root.")
else()
  set(XRSE_FACTORY_LUABIND_LUA_INCLUDE "${XRSE_FACTORY_LUABIND_INCLUDE_DIR}/luabind/lua_include.hpp")
  message(STATUS "XRSE_FACTORY_LUABIND_INCLUDE_DIR='${XRSE_FACTORY_LUABIND_INCLUDE_DIR}'")
  message(STATUS "xrSE_Factory luabind/lua_include.hpp: ${XRSE_FACTORY_LUABIND_LUA_INCLUDE}")
  if(NOT EXISTS "${XRSE_FACTORY_LUABIND_LUA_INCLUDE}")
    file(GLOB_RECURSE XRSE_FACTORY_LUABIND_LUA_INCLUDE_CANDIDATES
      "${XRSE_FACTORY_LUABIND_INCLUDE_DIR}/lua_include.hpp"
      "${XRSE_FACTORY_LUABIND_INCLUDE_DIR}/*/lua_include.hpp"
      "${XRSE_FACTORY_LUABIND_INCLUDE_DIR}/*/*/lua_include.hpp"
    )
    message(WARNING "XRSE_FACTORY_LUABIND_INCLUDE_DIR='${XRSE_FACTORY_LUABIND_INCLUDE_DIR}' does not contain luabind/lua_include.hpp. Candidate lua_include.hpp files under this path: ${XRSE_FACTORY_LUABIND_LUA_INCLUDE_CANDIDATES}")
  endif()
  set(XRSE_FACTORY_LUABIND_VERSION_HEADER "${XRSE_FACTORY_LUABIND_INCLUDE_DIR}/luabind/version.hpp")
  if(EXISTS "${XRSE_FACTORY_LUABIND_VERSION_HEADER}")
    file(STRINGS "${XRSE_FACTORY_LUABIND_VERSION_HEADER}" XRSE_FACTORY_LUABIND_VERSION_LINES REGEX "LUABIND_VERSION|090216")
    message(STATUS "xrSE_Factory luabind/version.hpp markers: ${XRSE_FACTORY_LUABIND_VERSION_LINES}")
    if(XRSE_FACTORY_LUABIND_VERSION_LINES MATCHES "LUABIND_VERSION[ \t]+900|090216")
      message(FATAL_ERROR "XRSE_FACTORY_LUABIND_INCLUDE_DIR points to luabind 0.9/current headers. Build 1935 uses Lua 5.0-era luabind; use the luabind 0.7 include root directly above luabind/lua_include.hpp.")
    endif()
  endif()
endif()

set(XRSE_FACTORY_LUABIND_OVERLOAD_REP_SOURCE "")
if(XRSE_FACTORY_LUABIND_INCLUDE_DIR)
  get_filename_component(XRSE_FACTORY_LUABIND_ROOT "${XRSE_FACTORY_LUABIND_INCLUDE_DIR}" DIRECTORY)
  foreach(candidate IN ITEMS
      "${XRSE_FACTORY_LUABIND_INCLUDE_DIR}/src/overload_rep.cpp"
      "${XRSE_FACTORY_LUABIND_ROOT}/src/overload_rep.cpp"
  )
    if(NOT XRSE_FACTORY_LUABIND_OVERLOAD_REP_SOURCE AND EXISTS "${candidate}")
      set(XRSE_FACTORY_LUABIND_OVERLOAD_REP_SOURCE "${candidate}")
    endif()
  endforeach()
  if(NOT XRSE_FACTORY_LUABIND_OVERLOAD_REP_SOURCE)
    message(WARNING "xrSE_Factory needs luabind::detail::overload_rep::call from luabind 0.7, but overload_rep.cpp was not found next to XRSE_FACTORY_LUABIND_INCLUDE_DIR.")
  endif()
endif()

if(NOT XRAY_BOOST_ROOT)
  message(WARNING "xrSE_Factory's legacy luabind sources usually require Boost 1.30.x-compatible headers. Set XRAY_BOOST_ROOT if they are not available through the compiler environment.")
else()
  message(STATUS "XRAY_BOOST_ROOT='${XRAY_BOOST_ROOT}'")
  set(XRSE_FACTORY_REQUIRED_BOOST_HEADER "${XRAY_BOOST_ROOT}/boost/type_traits/is_member_object_pointer.hpp")
  message(STATUS "xrSE_Factory required Boost header: ${XRSE_FACTORY_REQUIRED_BOOST_HEADER}")
  if(NOT EXISTS "${XRSE_FACTORY_REQUIRED_BOOST_HEADER}")
    message(WARNING "XRAY_BOOST_ROOT does not contain boost/type_traits/is_member_object_pointer.hpp. This Boost tree is incomplete or too old for the supplied legacy luabind headers.")
  endif()
endif()

set(XRSE_FACTORY_SOURCES
  "${XRSE_FACTORY_DIR}/ai_space.cpp"
  "${XRSE_FACTORY_DIR}/alife_event_personal.cpp"
  "${XRSE_FACTORY_DIR}/character_info.cpp"
  "${XRSE_FACTORY_DIR}/luabind/class.cpp"
  "${XRSE_FACTORY_DIR}/luabind/class_info.cpp"
  "${XRSE_FACTORY_DIR}/luabind/class_registry.cpp"
  "${XRSE_FACTORY_DIR}/luabind/class_rep.cpp"
  "${XRSE_FACTORY_DIR}/luabind/create_class.cpp"
  "${XRSE_FACTORY_DIR}/luabind/error.cpp"
  "${XRSE_FACTORY_DIR}/luabind/find_best_match.cpp"
  "${XRSE_FACTORY_DIR}/luabind/function.cpp"
  "${XRSE_FACTORY_DIR}/luabind/implicit_cast.cpp"
  "${XRSE_FACTORY_DIR}/luabind/link_compatibility.cpp"
  "${XRSE_FACTORY_DIR}/luabind/object_rep.cpp"
  "${XRSE_FACTORY_DIR}/luabind/open.cpp"
  "${XRSE_FACTORY_DIR}/luabind/pcall.cpp"
  "${XRSE_FACTORY_DIR}/luabind/ref.cpp"
  "${XRSE_FACTORY_DIR}/luabind/scope.cpp"
  "${XRSE_FACTORY_DIR}/luabind/stack_content_by_name.cpp"
  "${XRSE_FACTORY_DIR}/luabind/weak_ref.cpp"
  "${XRSE_FACTORY_DIR}/luabind/wrapper_base.cpp"
  ${XRSE_FACTORY_LUABIND_OVERLOAD_REP_SOURCE}
  "${XRSE_FACTORY_DIR}/object_factory.cpp"
  "${XRSE_FACTORY_DIR}/object_factory_register.cpp"
  "${XRSE_FACTORY_DIR}/object_factory_script.cpp"
  "${XRSE_FACTORY_DIR}/object_item_script.cpp"
  "${XRSE_FACTORY_DIR}/PHNetState.cpp"
  "${XRSE_FACTORY_DIR}/PHSynchronize.cpp"
  "${XRSE_FACTORY_DIR}/properties_list_helper_script.cpp"
  "${XRSE_FACTORY_DIR}/script_callStack.cpp"
  "${XRSE_FACTORY_DIR}/script_debugger.cpp"
  "${XRSE_FACTORY_DIR}/script_debugger_threads.cpp"
  "${XRSE_FACTORY_DIR}/script_debugger_utils.cpp"
  "${XRSE_FACTORY_DIR}/script_engine.cpp"
  "${XRSE_FACTORY_DIR}/script_engine_export.cpp"
  "${XRSE_FACTORY_DIR}/script_engine_script.cpp"
  "${XRSE_FACTORY_DIR}/script_file.cpp"
  "${XRSE_FACTORY_DIR}/script_flags_script.cpp"
  "${XRSE_FACTORY_DIR}/script_fmatrix_script.cpp"
  "${XRSE_FACTORY_DIR}/script_fvector_script.cpp"
  "${XRSE_FACTORY_DIR}/script_ini_file.cpp"
  "${XRSE_FACTORY_DIR}/script_ini_file_script.cpp"
  "${XRSE_FACTORY_DIR}/script_lua_helper.cpp"
  "${XRSE_FACTORY_DIR}/script_net_packet_script.cpp"
  "${XRSE_FACTORY_DIR}/script_process.cpp"
  "${XRSE_FACTORY_DIR}/script_properties_list_helper.cpp"
  "${XRSE_FACTORY_DIR}/script_rtoken_list_script.cpp"
  "${XRSE_FACTORY_DIR}/script_sound_type_script.cpp"
  "${XRSE_FACTORY_DIR}/script_stack_tracker.cpp"
  "${XRSE_FACTORY_DIR}/script_storage.cpp"
  "${XRSE_FACTORY_DIR}/script_thread.cpp"
  "${XRSE_FACTORY_DIR}/script_token_list.cpp"
  "${XRSE_FACTORY_DIR}/script_token_list_script.cpp"
  "${XRSE_FACTORY_DIR}/smart_cast.cpp"
  "${XRSE_FACTORY_DIR}/smart_cast_stats.cpp"
  "${XRSE_FACTORY_DIR}/stdafx.cpp"
  "${XRSE_FACTORY_DIR}/xrSE_Factory.cpp"
  "${XRSE_FACTORY_DIR}/xrServer_Factory.cpp"
  "${XRSE_FACTORY_DIR}/xrServer_Object_Base.cpp"
  "${XRSE_FACTORY_DIR}/xrServer_Objects.cpp"
  "${XRSE_FACTORY_DIR}/xrServer_Objects_Abstract.cpp"
  "${XRSE_FACTORY_DIR}/xrServer_Objects_ALife.cpp"
  "${XRSE_FACTORY_DIR}/xrServer_Objects_ALife_Items.cpp"
  "${XRSE_FACTORY_DIR}/xrServer_Objects_ALife_Items_script.cpp"
  "${XRSE_FACTORY_DIR}/xrServer_Objects_ALife_Items_script2.cpp"
  "${XRSE_FACTORY_DIR}/xrServer_Objects_ALife_Monsters.cpp"
  "${XRSE_FACTORY_DIR}/xrServer_Objects_ALife_Monsters_script.cpp"
  "${XRSE_FACTORY_DIR}/xrServer_Objects_ALife_Monsters_script2.cpp"
  "${XRSE_FACTORY_DIR}/xrServer_Objects_ALife_script.cpp"
  "${XRSE_FACTORY_DIR}/xrServer_Objects_ALife_script2.cpp"
  "${XRSE_FACTORY_DIR}/xrServer_Objects_script.cpp"
)

set(XRSE_FACTORY_LUABIND_SOURCES
  "${XRSE_FACTORY_DIR}/luabind/class.cpp"
  "${XRSE_FACTORY_DIR}/luabind/class_info.cpp"
  "${XRSE_FACTORY_DIR}/luabind/class_registry.cpp"
  "${XRSE_FACTORY_DIR}/luabind/class_rep.cpp"
  "${XRSE_FACTORY_DIR}/luabind/create_class.cpp"
  "${XRSE_FACTORY_DIR}/luabind/error.cpp"
  "${XRSE_FACTORY_DIR}/luabind/find_best_match.cpp"
  "${XRSE_FACTORY_DIR}/luabind/function.cpp"
  "${XRSE_FACTORY_DIR}/luabind/implicit_cast.cpp"
  "${XRSE_FACTORY_DIR}/luabind/link_compatibility.cpp"
  "${XRSE_FACTORY_DIR}/luabind/object_rep.cpp"
  "${XRSE_FACTORY_DIR}/luabind/open.cpp"
  "${XRSE_FACTORY_DIR}/luabind/pcall.cpp"
  "${XRSE_FACTORY_DIR}/luabind/ref.cpp"
  "${XRSE_FACTORY_DIR}/luabind/scope.cpp"
  "${XRSE_FACTORY_DIR}/luabind/stack_content_by_name.cpp"
  "${XRSE_FACTORY_DIR}/luabind/weak_ref.cpp"
  "${XRSE_FACTORY_DIR}/luabind/wrapper_base.cpp"
  ${XRSE_FACTORY_LUABIND_OVERLOAD_REP_SOURCE}
)

add_library(xrSE_Factory SHARED ${XRSE_FACTORY_SOURCES})

set_target_properties(xrSE_Factory PROPERTIES
  OUTPUT_NAME "xrSE_Factory"
  # xrSE_Factory.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${XRAY_LIBRARY_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(xrSE_Factory PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrSE_Factory
  PUBLIC
    "${XRSE_FACTORY_DIR}"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${XRSE_FACTORY_DIR}/luabind_compat"
    "${CMAKE_CURRENT_SOURCE_DIR}/xrLUA/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/xrLUA/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc"
)

if(XRSE_FACTORY_LUABIND_INCLUDE_DIR)
  target_include_directories(xrSE_Factory PRIVATE "${XRSE_FACTORY_LUABIND_INCLUDE_DIR}")
endif()

if(XRAY_LOKI_ROOT)
  target_include_directories(xrSE_Factory PRIVATE "${XRAY_LOKI_ROOT}")
endif()

if(XRAY_BOOST_ROOT)
  target_include_directories(xrSE_Factory PRIVATE "${XRAY_BOOST_ROOT}")
endif()

target_compile_definitions(xrSE_Factory
  PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    XRSE_FACTORY_EXPORTS
    MSVC
    # xrSE_Factory.vcproj sets ExceptionHandling="FALSE"; luabind 0.7 exposes
    # the error/cast callback API used by script_engine.cpp only in this mode.
    LUABIND_NO_EXCEPTIONS
    # xrSE_Factory includes xrCore.h, so it needs the same modern MSVC
    # compatibility switches for legacy xrCore/STL headers even though they
    # were implicit in the original VC2003 environment.
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Debug>:DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<CONFIG:Mixed>:DEBUG>
    $<$<CONFIG:Mixed>:MIXED>
    $<$<CONFIG:Mixed>:dSINGLE>
)

if(MSVC)
  target_compile_options(xrSE_Factory
    PRIVATE
      /nologo
      /W4
      /GR
      /Gz
      /EHs-
      /EHa-
  )

  # Mirrors xrSE_Factory.vcproj optimization/runtime choices where modern MSVC
  # still supports them; obsolete VC2003 settings such as /G7 and /Wp64 are
  # omitted.
  target_compile_options(xrSE_Factory PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Gy /Oi->
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy /Zc:wchar_t->
  )

  target_link_options(xrSE_Factory PRIVATE
    /nologo
    /MACHINE:I386
    # stdafx.cpp and xrXMLParser.h have original #pragma comment(lib, "x:\\...").
    # CMake links the corresponding targets directly, so ignore only those
    # hardcoded X: drive default library directives.
    /NODEFAULTLIB:x:\\xrCore.lib
    /NODEFAULTLIB:x:/xrCore.lib
    /NODEFAULTLIB:x:\\xrLUA.lib
    /NODEFAULTLIB:x:/xrLUA.lib
    /NODEFAULTLIB:x:\\xrXMLParser.lib
    /NODEFAULTLIB:x:/xrXMLParser.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL /MAP>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrSE_Factory PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XRSE_FACTORY_DIR}/stdafx.h>")
set_source_files_properties(${XRSE_FACTORY_LUABIND_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
if(MSVC)
  set_source_files_properties(${XRSE_FACTORY_LUABIND_SOURCES} PROPERTIES
    COMPILE_OPTIONS "/FI${XRSE_FACTORY_DIR}/luabind_compat/xray_luabind_compat.hpp"
  )
endif()

target_link_libraries(xrSE_Factory PRIVATE
  xrCore
  xrLUA
  xrXMLParser
  $<$<CONFIG:Mixed>:odbc32>
  $<$<CONFIG:Mixed>:odbccp32>
)

set(XRODE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xrODE")

set(XRODE_SOURCES
  "${XRODE_DIR}/ode/src/array.cpp"
  "${XRODE_DIR}/ode/src/Bounder33.cpp"
  "${XRODE_DIR}/ode/src/collision_kernel.cpp"
  "${XRODE_DIR}/ode/src/collision_quadtreespace.cpp"
  "${XRODE_DIR}/ode/src/collision_space.cpp"
  "${XRODE_DIR}/ode/src/collision_std.cpp"
  "${XRODE_DIR}/ode/src/collision_transform.cpp"
  "${XRODE_DIR}/ode/src/collision_util.cpp"
  "${XRODE_DIR}/ode/src/error.cpp"
  "${XRODE_DIR}/ode/src/export-dif.cpp"
  "${XRODE_DIR}/ode/src/fastdot.c"
  "${XRODE_DIR}/ode/src/fastldlt.c"
  "${XRODE_DIR}/ode/src/fastlsolve.c"
  "${XRODE_DIR}/ode/src/fastltsolve.c"
  "${XRODE_DIR}/ode/src/joint.cpp"
  "${XRODE_DIR}/ode/src/lcp.cpp"
  "${XRODE_DIR}/ode/src/mass.cpp"
  "${XRODE_DIR}/ode/src/mat.cpp"
  "${XRODE_DIR}/ode/src/matrix.cpp"
  "${XRODE_DIR}/ode/src/memory.cpp"
  "${XRODE_DIR}/ode/src/misc.cpp"
  "${XRODE_DIR}/ode/src/obstack.cpp"
  "${XRODE_DIR}/ode/src/ode.cpp"
  "${XRODE_DIR}/ode/src/odemath.cpp"
  "${XRODE_DIR}/ode/src/quickstep.cpp"
  "${XRODE_DIR}/ode/src/rotation.cpp"
  "${XRODE_DIR}/ode/src/step.cpp"
  "${XRODE_DIR}/ode/src/stepfast.cpp"
  "${XRODE_DIR}/ode/src/StepJointInternal.cpp"
  "${XRODE_DIR}/ode/src/testing.cpp"
  "${XRODE_DIR}/ode/src/timer.cpp"
  "${XRODE_DIR}/ode/src/util.cpp"
)

add_library(ode SHARED ${XRODE_SOURCES})

set_target_properties(ode PROPERTIES
  OUTPUT_NAME "ode"
  # ode_default.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${XRAY_LIBRARY_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(ode PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(ode
  PUBLIC
    "${XRODE_DIR}/include"
)

target_compile_definitions(ode
  PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    MSVC
    dSINGLE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<NOT:$<CONFIG:Debug>>:dNODEBUG>
)

if(MSVC)
  target_compile_options(ode
    PRIVATE
      /nologo
      /W3
      /Gz
      /EHs-
      /EHa-
      $<$<CONFIG:Debug>:/Od /Zi /RTC1>
      $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
      $<$<CONFIG:Release>:/Oy>
  )

  target_link_options(ode PRIVATE
    /nologo
    /DEF:${XRODE_DIR}/config/msvcdefs.def
    /MACHINE:I386
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

# ode_default.vcproj contains a regsvr32 post-build event, but it is marked
# ExcludedFromBuild="TRUE" in all original configurations. Keep registration
# outside the default CMake build.

set(XRNETSERVER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xrNetServer")

if(NOT XRAY_EFFECTIVE_DXSDK_ROOT)
  message(WARNING "xrNetServer includes legacy DirectPlay header dplay8.h. Set XRAY_DXSDK_ROOT, DXSDK_DIR, or DIRECTX_SDK_DIR to a DirectX 9 SDK root if the active Windows SDK does not provide DirectPlay headers.")
elseif(NOT EXISTS "${XRAY_EFFECTIVE_DXSDK_ROOT}/Include/dplay8.h" AND NOT EXISTS "${XRAY_EFFECTIVE_DXSDK_ROOT}/include/dplay8.h")
  message(WARNING "xrNetServer needs dplay8.h, but it was not found under XRAY_EFFECTIVE_DXSDK_ROOT='${XRAY_EFFECTIVE_DXSDK_ROOT}'.")
endif()

set(XRNETSERVER_SOURCES
  "${XRNETSERVER_DIR}/NET_Client.cpp"
  "${XRNETSERVER_DIR}/NET_Compressor.cpp"
  "${XRNETSERVER_DIR}/NET_Server.cpp"
  "${XRNETSERVER_DIR}/stdafx.cpp"
)

add_library(xrNetServer SHARED ${XRNETSERVER_SOURCES})

set_target_properties(xrNetServer PROPERTIES
  OUTPUT_NAME "xrNetServer"
  # xrNetServer.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${XRAY_LIBRARY_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(xrNetServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrNetServer
  PUBLIC
    "${XRNETSERVER_DIR}"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc"
)

if(XRAY_EFFECTIVE_DXSDK_ROOT)
  target_include_directories(xrNetServer PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Include" "${XRAY_EFFECTIVE_DXSDK_ROOT}/include")
  target_link_directories(xrNetServer PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib/x86" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib/x86")
endif()

target_compile_definitions(xrNetServer
  PRIVATE
    WIN32
    _WINDOWS
    XR_NETSERVER_EXPORTS
    # xrNetServer includes xrCore.h, so it needs the same modern MSVC
    # compatibility switches for legacy xrCore/STL headers even though they
    # were implicit in the original VC2003 environment.
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xrNetServer
    PRIVATE
      /nologo
      /W4
      /GR
      /Gz
      /EHs-
      /EHa-
  )

  # Mirrors xrNetServer.vcproj optimization/runtime choices where modern MSVC
  # still supports them; obsolete VC2003 settings such as /G7 and /Wp64 are
  # omitted.
  target_compile_options(xrNetServer PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Oi->
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy>
  )

  target_link_options(xrNetServer PRIVATE
    /nologo
    # stdafx.cpp and NET_Shared.h have original #pragma comment(lib, "x:\\...").
    # CMake links xrCore directly and builds xrNetServer here, so ignore only
    # those hardcoded X: drive default library directives.
    /NODEFAULTLIB:x:\\xrCore.lib
    /NODEFAULTLIB:x:\\xrNetServer.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrNetServer PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XRNETSERVER_DIR}/stdafx.h>")

target_link_libraries(xrNetServer PRIVATE
  xrCore
  odbc32
  odbccp32
  dxguid
)

set(XRSOUND_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xr_3da/xrSound")
set(XRAY_OGG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/External Library/OGG" CACHE PATH "Legacy Ogg/Vorbis source or SDK root for xrSound")
set(XRAY_OPENAL_ROOT "" CACHE PATH "Legacy OpenAL SDK root containing AL/al.h, AL/alc.h, AL/alut.h and wrap_oal.lib for xrSound")
set(XRAY_WRAP_OAL_DLL "C:/Windows/SysWOW64/wrap_oal.dll" CACHE FILEPATH "Installed 32-bit OpenAL wrapper runtime used to generate an xrSound import library when wrap_oal.lib is missing")
set(XRAY_EAX_ROOT "" CACHE PATH "Legacy Creative EAX SDK root containing eax.h and eax.lib for xrSound")

set(XRSOUND_OGG_INCLUDE_DIR "")
set(XRSOUND_OGG_LIB_DEBUG "")
set(XRSOUND_OGG_LIB_RELEASE "")
set(XRSOUND_VORBIS_LIB_DEBUG "")
set(XRSOUND_VORBIS_LIB_RELEASE "")
set(XRSOUND_VORBISFILE_LIB_DEBUG "")
set(XRSOUND_VORBISFILE_LIB_RELEASE "")
if(EXISTS "${XRAY_OGG_ROOT}/include/vorbis/vorbisfile.h" AND EXISTS "${XRAY_OGG_ROOT}/include/ogg/ogg.h" AND EXISTS "${XRAY_OGG_ROOT}/lib/Win32/vorbisfile.lib")
  set(XRSOUND_OGG_INCLUDE_DIR "${XRAY_OGG_ROOT}/include")
  set(XRSOUND_OGG_LIB_DEBUG "${XRAY_OGG_ROOT}/lib/Win32/ogg.lib")
  set(XRSOUND_OGG_LIB_RELEASE "${XRAY_OGG_ROOT}/lib/Win32/ogg.lib")
  set(XRSOUND_VORBIS_LIB_DEBUG "${XRAY_OGG_ROOT}/lib/Win32/vorbis.lib")
  set(XRSOUND_VORBIS_LIB_RELEASE "${XRAY_OGG_ROOT}/lib/Win32/vorbis.lib")
  set(XRSOUND_VORBISFILE_LIB_DEBUG "${XRAY_OGG_ROOT}/lib/Win32/vorbisfile.lib")
  set(XRSOUND_VORBISFILE_LIB_RELEASE "${XRAY_OGG_ROOT}/lib/Win32/vorbisfile.lib")
elseif(EXISTS "${XRAY_OGG_ROOT}/Include/vorbis/vorbisfile.h" AND EXISTS "${XRAY_OGG_ROOT}/Include/ogg/ogg.h")
  set(XRSOUND_OGG_INCLUDE_DIR "${XRAY_OGG_ROOT}/Include")
  set(XRSOUND_OGG_LIB_DEBUG "${XRAY_OGG_ROOT}/Libs/ogg_static_d.lib")
  set(XRSOUND_OGG_LIB_RELEASE "${XRAY_OGG_ROOT}/Libs/ogg_static.lib")
  set(XRSOUND_VORBIS_LIB_DEBUG "${XRAY_OGG_ROOT}/Libs/vorbis_static_d.lib")
  set(XRSOUND_VORBIS_LIB_RELEASE "${XRAY_OGG_ROOT}/Libs/vorbis_static.lib")
  set(XRSOUND_VORBISFILE_LIB_DEBUG "${XRAY_OGG_ROOT}/Libs/vorbisfile_static_d.lib")
  set(XRSOUND_VORBISFILE_LIB_RELEASE "${XRAY_OGG_ROOT}/Libs/vorbisfile_static.lib")
elseif(EXISTS "${XRAY_OGG_ROOT}/vorbis/include/vorbis/vorbisfile.h" AND EXISTS "${XRAY_OGG_ROOT}/ogg/include/ogg/ogg.h")
  set(XRSOUND_OGG_INCLUDE_DIR "${XRAY_OGG_ROOT}/ogg/include" "${XRAY_OGG_ROOT}/vorbis/include")
else()
  message(WARNING "xrSound needs Ogg/Vorbis headers. Set XRAY_OGG_ROOT to either the Xiph root containing Include/ and Libs/, or the legacy source root containing ogg/include and vorbis/include.")
endif()
foreach(required_lib IN ITEMS
    XRSOUND_OGG_LIB_DEBUG
    XRSOUND_OGG_LIB_RELEASE
    XRSOUND_VORBIS_LIB_DEBUG
    XRSOUND_VORBIS_LIB_RELEASE
    XRSOUND_VORBISFILE_LIB_DEBUG
    XRSOUND_VORBISFILE_LIB_RELEASE
)
  if(${required_lib} AND NOT EXISTS "${${required_lib}}")
    message(WARNING "xrSound expected ${required_lib}='${${required_lib}}', but that file does not exist.")
  endif()
endforeach()
if(NOT XRSOUND_VORBIS_LIB_RELEASE AND NOT XRSOUND_VORBISFILE_LIB_RELEASE)
  message(WARNING "xrSound original pragmas request vorbis.lib and vorbisfile.lib. Configure XRAY_OGG_ROOT to a Xiph binary root or provide compatible libraries.")
endif()
if(NOT XRAY_OPENAL_ROOT)
  message(WARNING "xrSound needs legacy OpenAL headers AL/al.h, AL/alc.h, AL/alut.h and wrap_oal.lib. Set XRAY_OPENAL_ROOT to the original OpenAL SDK root.")
else()
  set(XRSOUND_GENERATED_OPENAL_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/compat/openal/include")
  set(XRSOUND_OPENAL_AL_H "")
  set(XRSOUND_OPENAL_ALC_H "")
  set(XRSOUND_OPENAL_ALUT_H "")
  foreach(candidate IN ITEMS
      "${XRAY_OPENAL_ROOT}/include/AL/al.h"
      "${XRAY_OPENAL_ROOT}/Include/AL/al.h"
      "${XRAY_OPENAL_ROOT}/AL/al.h"
  )
    if(NOT XRSOUND_OPENAL_AL_H AND EXISTS "${candidate}")
      set(XRSOUND_OPENAL_AL_H "${candidate}")
    endif()
  endforeach()
  foreach(candidate IN ITEMS
      "${XRAY_OPENAL_ROOT}/include/AL/alc.h"
      "${XRAY_OPENAL_ROOT}/Include/AL/alc.h"
      "${XRAY_OPENAL_ROOT}/AL/alc.h"
  )
    if(NOT XRSOUND_OPENAL_ALC_H AND EXISTS "${candidate}")
      set(XRSOUND_OPENAL_ALC_H "${candidate}")
    endif()
  endforeach()
  foreach(candidate IN ITEMS
      "${XRAY_OPENAL_ROOT}/include/AL/alut.h"
      "${XRAY_OPENAL_ROOT}/Include/AL/alut.h"
      "${XRAY_OPENAL_ROOT}/AL/alut.h"
  )
    if(NOT XRSOUND_OPENAL_ALUT_H AND EXISTS "${candidate}")
      set(XRSOUND_OPENAL_ALUT_H "${candidate}")
    endif()
  endforeach()
  if(NOT XRSOUND_OPENAL_AL_H)
    if(EXISTS "${XRAY_OPENAL_ROOT}/include/al.h" OR EXISTS "${XRAY_OPENAL_ROOT}/Include/al.h")
      message(WARNING "XRAY_OPENAL_ROOT='${XRAY_OPENAL_ROOT}' has flat OpenAL headers, but xrSound includes <AL/al.h>. Generating a build-tree include overlay for xrSound.")
      file(MAKE_DIRECTORY "${XRSOUND_GENERATED_OPENAL_INCLUDE_DIR}/AL")
      if(EXISTS "${XRAY_OPENAL_ROOT}/include/al.h")
        set(XRSOUND_OPENAL_FLAT_AL_H_SOURCE "${XRAY_OPENAL_ROOT}/include/al.h")
      else()
        set(XRSOUND_OPENAL_FLAT_AL_H_SOURCE "${XRAY_OPENAL_ROOT}/Include/al.h")
      endif()
      file(TO_CMAKE_PATH "${XRSOUND_OPENAL_FLAT_AL_H_SOURCE}" XRSOUND_OPENAL_FLAT_AL_H)
      file(WRITE "${XRSOUND_GENERATED_OPENAL_INCLUDE_DIR}/AL/al.h" "#pragma once\n#include \"${XRSOUND_OPENAL_FLAT_AL_H}\"\n")
      set(XRSOUND_OPENAL_AL_H "${XRSOUND_GENERATED_OPENAL_INCLUDE_DIR}/AL/al.h")
    else()
      message(WARNING "XRAY_OPENAL_ROOT='${XRAY_OPENAL_ROOT}' does not contain AL/al.h.")
    endif()
  endif()
  if(NOT XRSOUND_OPENAL_ALC_H AND (EXISTS "${XRAY_OPENAL_ROOT}/include/alc.h" OR EXISTS "${XRAY_OPENAL_ROOT}/Include/alc.h"))
    file(MAKE_DIRECTORY "${XRSOUND_GENERATED_OPENAL_INCLUDE_DIR}/AL")
    if(EXISTS "${XRAY_OPENAL_ROOT}/include/alc.h")
      set(XRSOUND_OPENAL_FLAT_ALC_H_SOURCE "${XRAY_OPENAL_ROOT}/include/alc.h")
    else()
      set(XRSOUND_OPENAL_FLAT_ALC_H_SOURCE "${XRAY_OPENAL_ROOT}/Include/alc.h")
    endif()
    file(TO_CMAKE_PATH "${XRSOUND_OPENAL_FLAT_ALC_H_SOURCE}" XRSOUND_OPENAL_FLAT_ALC_H)
    file(WRITE "${XRSOUND_GENERATED_OPENAL_INCLUDE_DIR}/AL/alc.h" "#pragma once\n#include \"${XRSOUND_OPENAL_FLAT_ALC_H}\"\n")
    set(XRSOUND_OPENAL_ALC_H "${XRSOUND_GENERATED_OPENAL_INCLUDE_DIR}/AL/alc.h")
  endif()
  if(NOT XRSOUND_OPENAL_ALUT_H AND (EXISTS "${XRAY_OPENAL_ROOT}/include/alut.h" OR EXISTS "${XRAY_OPENAL_ROOT}/Include/alut.h"))
    file(MAKE_DIRECTORY "${XRSOUND_GENERATED_OPENAL_INCLUDE_DIR}/AL")
    if(EXISTS "${XRAY_OPENAL_ROOT}/include/alut.h")
      set(XRSOUND_OPENAL_FLAT_ALUT_H_SOURCE "${XRAY_OPENAL_ROOT}/include/alut.h")
    else()
      set(XRSOUND_OPENAL_FLAT_ALUT_H_SOURCE "${XRAY_OPENAL_ROOT}/Include/alut.h")
    endif()
    file(TO_CMAKE_PATH "${XRSOUND_OPENAL_FLAT_ALUT_H_SOURCE}" XRSOUND_OPENAL_FLAT_ALUT_H)
    file(WRITE "${XRSOUND_GENERATED_OPENAL_INCLUDE_DIR}/AL/alut.h" "#pragma once\n#include \"${XRSOUND_OPENAL_FLAT_ALUT_H}\"\n")
    set(XRSOUND_OPENAL_ALUT_H "${XRSOUND_GENERATED_OPENAL_INCLUDE_DIR}/AL/alut.h")
  endif()
  if(NOT XRSOUND_OPENAL_ALUT_H)
    message(WARNING "xrSound includes <AL/alut.h>, but XRAY_OPENAL_ROOT='${XRAY_OPENAL_ROOT}' does not provide alut.h. Provide the matching legacy OpenAL/ALUT headers.")
  endif()

  set(XRSOUND_OPENAL_WRAP_OAL_LIB "")
  foreach(candidate IN ITEMS
      "${XRAY_OPENAL_ROOT}/libs/Win32/wrap_oal.lib"
      "${XRAY_OPENAL_ROOT}/lib/Win32/wrap_oal.lib"
      "${XRAY_OPENAL_ROOT}/lib/wrap_oal.lib"
      "${XRAY_OPENAL_ROOT}/wrap_oal.lib"
  )
    if(NOT XRSOUND_OPENAL_WRAP_OAL_LIB AND EXISTS "${candidate}")
      set(XRSOUND_OPENAL_WRAP_OAL_LIB "${candidate}")
    endif()
  endforeach()
  if(NOT XRSOUND_OPENAL_WRAP_OAL_LIB)
    if(EXISTS "${XRAY_OPENAL_ROOT}/libs/Win32/OpenAL32.lib" OR EXISTS "${XRAY_OPENAL_ROOT}/lib/Win32/OpenAL32.lib" OR EXISTS "${XRAY_OPENAL_ROOT}/lib/OpenAL32.lib" OR EXISTS "${XRAY_OPENAL_ROOT}/OpenAL32.lib")
      message(WARNING "XRAY_OPENAL_ROOT='${XRAY_OPENAL_ROOT}' contains OpenAL32.lib, but original xrSound explicitly links wrap_oal.lib. OpenAL32.lib is not substituted; provide the original wrap_oal.lib.")
    else()
      message(WARNING "XRAY_OPENAL_ROOT='${XRAY_OPENAL_ROOT}' does not contain wrap_oal.lib required by the original xrSound pragmas.")
    endif()
    if(MSVC AND EXISTS "${XRAY_WRAP_OAL_DLL}")
      set(XRSOUND_OPENAL_WRAP_OAL_LIB "${CMAKE_CURRENT_BINARY_DIR}/compat/openal/wrap_oal.lib")
      set(XRSOUND_WRAP_OAL_DEF "${CMAKE_CURRENT_BINARY_DIR}/compat/openal/wrap_oal.def")
      file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/compat/openal")
      file(WRITE "${XRSOUND_WRAP_OAL_DEF}" "LIBRARY wrap_oal.dll\nEXPORTS\n  EAXGet\n  EAXSet\n  alBuffer3f\n  alBuffer3i\n  alBufferData\n  alBufferf\n  alBufferfv\n  alBufferi\n  alBufferiv\n  alDeleteBuffers\n  alDeleteSources\n  alDisable\n  alDistanceModel\n  alDopplerFactor\n  alDopplerVelocity\n  alEnable\n  alGenBuffers\n  alGenSources\n  alGetBoolean\n  alGetBooleanv\n  alGetBuffer3f\n  alGetBuffer3i\n  alGetBufferf\n  alGetBufferfv\n  alGetBufferi\n  alGetBufferiv\n  alGetDouble\n  alGetDoublev\n  alGetEnumValue\n  alGetError\n  alGetFloat\n  alGetFloatv\n  alGetInteger\n  alGetIntegerv\n  alGetListener3f\n  alGetListener3i\n  alGetListenerf\n  alGetListenerfv\n  alGetListeneri\n  alGetListeneriv\n  alGetProcAddress\n  alGetSource3f\n  alGetSource3i\n  alGetSourcef\n  alGetSourcefv\n  alGetSourcei\n  alGetSourceiv\n  alGetString\n  alIsBuffer\n  alIsEnabled\n  alIsExtensionPresent\n  alIsSource\n  alListener3f\n  alListener3i\n  alListenerf\n  alListenerfv\n  alListeneri\n  alListeneriv\n  alSource3f\n  alSource3i\n  alSourcePause\n  alSourcePausev\n  alSourcePlay\n  alSourcePlayv\n  alSourceQueueBuffers\n  alSourceRewind\n  alSourceRewindv\n  alSourceStop\n  alSourceStopv\n  alSourceUnqueueBuffers\n  alSourcef\n  alSourcefv\n  alSourcei\n  alSourceiv\n  alSpeedOfSound\n  alcCaptureCloseDevice\n  alcCaptureOpenDevice\n  alcCaptureSamples\n  alcCaptureStart\n  alcCaptureStop\n  alcCloseDevice\n  alcCreateContext\n  alcDestroyContext\n  alcGetContextsDevice\n  alcGetCurrentContext\n  alcGetEnumValue\n  alcGetError\n  alcGetIntegerv\n  alcGetProcAddress\n  alcGetString\n  alcIsExtensionPresent\n  alcMakeContextCurrent\n  alcOpenDevice\n  alcProcessContext\n  alcSuspendContext\n")
      execute_process(
        COMMAND "${CMAKE_AR}" /nologo /machine:x86 /def:${XRSOUND_WRAP_OAL_DEF} /out:${XRSOUND_OPENAL_WRAP_OAL_LIB}
        RESULT_VARIABLE XRSOUND_WRAP_OAL_LIB_RESULT
      )
      if(XRSOUND_WRAP_OAL_LIB_RESULT EQUAL 0)
        message(STATUS "xrSound generated wrap_oal import library: ${XRSOUND_OPENAL_WRAP_OAL_LIB} from ${XRAY_WRAP_OAL_DLL}")
      else()
        message(WARNING "Failed to generate xrSound wrap_oal import library from '${XRAY_WRAP_OAL_DLL}'.")
        set(XRSOUND_OPENAL_WRAP_OAL_LIB "")
      endif()
    endif()
  endif()
endif()
if(NOT XRAY_EAX_ROOT)
  message(WARNING "xrSound needs legacy EAX headers/libs: eax.h and eax.lib. Set XRAY_EAX_ROOT to the original EAX SDK root.")
elseif(NOT EXISTS "${XRAY_EAX_ROOT}/include/eax.h" AND NOT EXISTS "${XRAY_EAX_ROOT}/eax.h")
  message(WARNING "XRAY_EAX_ROOT='${XRAY_EAX_ROOT}' does not contain eax.h.")
endif()

set(XRSOUND_SOURCES
  "${XRSOUND_DIR}/guids.cpp"
  "${XRSOUND_DIR}/sound.cpp"
  "${XRSOUND_DIR}/stdafx.cpp"
  "${XRSOUND_DIR}/SoundRender_Core.cpp"
  "${XRSOUND_DIR}/SoundRender_Core_Processor.cpp"
  "${XRSOUND_DIR}/SoundRender_Core_SourceManager.cpp"
  "${XRSOUND_DIR}/SoundRender_Core_StartStop.cpp"
  "${XRSOUND_DIR}/SoundRender_CoreA.cpp"
  "${XRSOUND_DIR}/SoundRender_CoreD.cpp"
  "${XRSOUND_DIR}/SoundRender_Emitter.cpp"
  "${XRSOUND_DIR}/SoundRender_Emitter_FSM.cpp"
  "${XRSOUND_DIR}/SoundRender_Emitter_StartStop.cpp"
  "${XRSOUND_DIR}/SoundRender_Emitter_streamer.cpp"
  "${XRSOUND_DIR}/SoundRender_Source.cpp"
  "${XRSOUND_DIR}/SoundRender_Source_loader.cpp"
  "${XRSOUND_DIR}/SoundRender_Target.cpp"
  "${XRSOUND_DIR}/SoundRender_TargetA.cpp"
  "${XRSOUND_DIR}/SoundRender_TargetD.cpp"
  "${XRSOUND_DIR}/SoundRender_Environment.cpp"
  "${XRSOUND_DIR}/SoundRender_Cache.cpp"
)

add_library(xrSound SHARED ${XRSOUND_SOURCES})

set_target_properties(xrSound PROPERTIES
  OUTPUT_NAME "xrSound"
  # xrSound.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
  VS_GLOBAL_CharacterSet "MultiByte"
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
  set_target_properties(xrSound PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrSound
  PRIVATE
    "${XRSOUND_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/xr_3da/xrCDB"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc"
    ${XRSOUND_OGG_INCLUDE_DIR}
)

if(XRAY_OPENAL_ROOT)
  target_include_directories(xrSound PRIVATE "${XRSOUND_GENERATED_OPENAL_INCLUDE_DIR}" "${XRAY_OPENAL_ROOT}/include" "${XRAY_OPENAL_ROOT}/Include" "${XRAY_OPENAL_ROOT}")
  target_link_directories(xrSound PRIVATE "${XRAY_OPENAL_ROOT}/libs/Win32" "${XRAY_OPENAL_ROOT}/lib/Win32" "${XRAY_OPENAL_ROOT}/lib" "${XRAY_OPENAL_ROOT}/libs" "${XRAY_OPENAL_ROOT}")
endif()

if(XRAY_EAX_ROOT)
  target_include_directories(xrSound PRIVATE "${XRAY_EAX_ROOT}/include" "${XRAY_EAX_ROOT}")
  target_link_directories(xrSound PRIVATE "${XRAY_EAX_ROOT}/lib" "${XRAY_EAX_ROOT}")
endif()

if(XRAY_EFFECTIVE_DXSDK_ROOT)
  target_include_directories(xrSound PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Include" "${XRAY_EFFECTIVE_DXSDK_ROOT}/include")
  target_link_directories(xrSound PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib/x86" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib/x86")
endif()

target_compile_definitions(xrSound
  PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    XRSOUND_EXPORTS
    # xrSound includes xrCore.h, so it needs the same modern MSVC compatibility
    # switches for legacy xrCore/STL headers even though they were implicit in
    # the original VC2003 environment.
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xrSound
    PRIVATE
      /nologo
      /W4
      /GR
      /Gz
      /EHs-
      /EHa-
  )

  # Mirrors xrSound.vcproj optimization/runtime choices where modern MSVC still
  # supports them; obsolete VC2003 settings such as /G7 and /Wp64 are omitted.
  target_compile_options(xrSound PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Gy /Oi->
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy>
  )

  target_link_options(xrSound PRIVATE
    /nologo
    /SUBSYSTEM:WINDOWS
    /MACHINE:I386
    # Legacy Xiph static libraries were built without SAFESEH metadata.
    /SAFESEH:NO
    # stdafx.cpp has original hardcoded X: drive pragma libraries. CMake links
    # the already-ported targets directly and resolves third-party audio libs
    # through cache-configured legacy SDK paths.
    /NODEFAULTLIB:x:\\xrCore
    /NODEFAULTLIB:x:\\xrCore.lib
    /NODEFAULTLIB:x:\\xrCDB
    /NODEFAULTLIB:x:\\xrCDB.lib
    /NODEFAULTLIB:x:\\eax.lib
    /NODEFAULTLIB:x:\\vorbis.lib
    /NODEFAULTLIB:x:\\vorbisfile.lib
    /NODEFAULTLIB:vorbis.lib
    /NODEFAULTLIB:vorbisfile.lib
    /NODEFAULTLIB:x:\\wrap_oal.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL /MAP>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrSound PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XRSOUND_DIR}/stdafx.h>")
set_source_files_properties("${XRSOUND_DIR}/stdafx.cpp" PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
set_source_files_properties("${XRSOUND_DIR}/guids.cpp" PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
set_source_files_properties(
  "${XRSOUND_DIR}/SoundRender_Source.cpp"
  "${XRSOUND_DIR}/SoundRender_Source_loader.cpp"
  PROPERTIES SKIP_PRECOMPILE_HEADERS ON
)
if(MSVC)
  # The original xrSound project uses /Gz globally, but this Xiph Vorbisfile
  # header has plain extern prototypes while the linked static libs export
  # cdecl symbols. Keep /Gz for xrSound itself and compile only the Vorbis
  # call sites with /Gd so their declarations match the Xiph library ABI.
  set_source_files_properties(
    "${XRSOUND_DIR}/SoundRender_Source.cpp"
    "${XRSOUND_DIR}/SoundRender_Source_loader.cpp"
    PROPERTIES COMPILE_OPTIONS /Gd
  )
endif()

target_link_libraries(xrSound PRIVATE
  xrCore
  xrCDB
  eax
  $<$<CONFIG:Debug>:${XRSOUND_OGG_LIB_DEBUG}>
  $<$<NOT:$<CONFIG:Debug>>:${XRSOUND_OGG_LIB_RELEASE}>
  $<$<CONFIG:Debug>:${XRSOUND_VORBIS_LIB_DEBUG}>
  $<$<NOT:$<CONFIG:Debug>>:${XRSOUND_VORBIS_LIB_RELEASE}>
  $<$<CONFIG:Debug>:${XRSOUND_VORBISFILE_LIB_DEBUG}>
  $<$<NOT:$<CONFIG:Debug>>:${XRSOUND_VORBISFILE_LIB_RELEASE}>
  ${XRSOUND_OPENAL_WRAP_OAL_LIB}
  dsound
  dxguid
  msacm32
)

set(XR3DA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xr_3da")

if(NOT XRAY_EFFECTIVE_DXSDK_ROOT)
  message(WARNING "XR_3DA includes legacy DirectX 9 headers such as d3d9.h, dplay8.h, dinput.h, and d3dx9.h. Set XRAY_DXSDK_ROOT, D3DXSDK_DIR, DXSDK_DIR, or DIRECTX_SDK_DIR to the original DirectX SDK root if the active SDK does not provide them.")
elseif(NOT EXISTS "${XRAY_EFFECTIVE_DXSDK_ROOT}/Include/d3dx9.h" AND NOT EXISTS "${XRAY_EFFECTIVE_DXSDK_ROOT}/include/d3dx9.h")
  message(WARNING "XR_3DA needs d3dx9.h, but it was not found under XRAY_EFFECTIVE_DXSDK_ROOT='${XRAY_EFFECTIVE_DXSDK_ROOT}'.")
endif()

set(XR3DA_SOURCES
  "${XR3DA_DIR}/defines.cpp"
  "${XR3DA_DIR}/stdafx.cpp"
  "${XR3DA_DIR}/x_ray.cpp"
  "${XR3DA_DIR}/resource.rc"
  "${XR3DA_DIR}/IInputReceiver.cpp"
  "${XR3DA_DIR}/Xr_input.cpp"
  "${XR3DA_DIR}/ISheduled.cpp"
  "${XR3DA_DIR}/xrSheduler.cpp"
  "${XR3DA_DIR}/IRenderable.cpp"
  "${XR3DA_DIR}/Render.cpp"
  "${XR3DA_DIR}/ISpatial.cpp"
  "${XR3DA_DIR}/ISpatial_q_box.cpp"
  "${XR3DA_DIR}/ISpatial_q_frustum.cpp"
  "${XR3DA_DIR}/ISpatial_q_ray.cpp"
  "${XR3DA_DIR}/ISpatial_verify.cpp"
  "${XR3DA_DIR}/ICollidable.cpp"
  "${XR3DA_DIR}/IGame_Level.cpp"
  "${XR3DA_DIR}/IGame_Level_check_textures.cpp"
  "${XR3DA_DIR}/IGame_Persistent.cpp"
  "${XR3DA_DIR}/IGame_ObjectPool.cpp"
  "${XR3DA_DIR}/xr_object.cpp"
  "${XR3DA_DIR}/xr_object_list.cpp"
  "${XR3DA_DIR}/CustomHUD.cpp"
  "${XR3DA_DIR}/Environment.cpp"
  "${XR3DA_DIR}/Rain.cpp"
  "${XR3DA_DIR}/thunderbolt.cpp"
  "${XR3DA_DIR}/xr_efflensflare.cpp"
  "${XR3DA_DIR}/CameraBase.cpp"
  "${XR3DA_DIR}/CameraManager.cpp"
  "${XR3DA_DIR}/Effector.cpp"
  "${XR3DA_DIR}/EffectorFall.cpp"
  "${XR3DA_DIR}/EffectorPP.cpp"
  "${XR3DA_DIR}/FDemoPlay.cpp"
  "${XR3DA_DIR}/FDemoRecord.cpp"
  "${XR3DA_DIR}/xr_area.cpp"
  "${XR3DA_DIR}/xr_area_movement.cpp"
  "${XR3DA_DIR}/xr_area_query.cpp"
  "${XR3DA_DIR}/xr_area_raypick.cpp"
  "${XR3DA_DIR}/xr_collide_form.cpp"
  "${XR3DA_DIR}/Envelope.cpp"
  "${XR3DA_DIR}/interp.cpp"
  "${XR3DA_DIR}/motion.cpp"
  "${XR3DA_DIR}/ObjectAnimator.cpp"
  "${XR3DA_DIR}/Feel_Touch.cpp"
  "${XR3DA_DIR}/Feel_Vision.cpp"
  "${XR3DA_DIR}/FBasicVisual.cpp"
  "${XR3DA_DIR}/fmesh.cpp"
  "${XR3DA_DIR}/FVisual.cpp"
  "${XR3DA_DIR}/FProgressive.cpp"
  "${XR3DA_DIR}/FProgressiveFixedVisual.cpp"
  "${XR3DA_DIR}/FHierrarhyVisual.cpp"
  # FastSprite.cpp and FastSpriteRail.cpp are excluded in XR_3DA.vcproj.
  "${XR3DA_DIR}/FCached.cpp"
  "${XR3DA_DIR}/ParticleCustom.cpp"
  "${XR3DA_DIR}/SkeletonAnimated.cpp"
  "${XR3DA_DIR}/SkeletonCustom.cpp"
  "${XR3DA_DIR}/SkeletonRigid.cpp"
  "${XR3DA_DIR}/SkeletonX.cpp"
  "${XR3DA_DIR}/FLOD.cpp"
  "${XR3DA_DIR}/Frustum.cpp"
  "${XR3DA_DIR}/PS_instance.cpp"
  "${XR3DA_DIR}/LightAnimLibrary.cpp"
  "${XR3DA_DIR}/GameFont.cpp"
  "${XR3DA_DIR}/device.cpp"
  "${XR3DA_DIR}/Device_create.cpp"
  "${XR3DA_DIR}/Device_destroy.cpp"
  "${XR3DA_DIR}/Device_Initialize.cpp"
  "${XR3DA_DIR}/Device_Misc.cpp"
  "${XR3DA_DIR}/Device_overdraw.cpp"
  "${XR3DA_DIR}/Device_wndproc.cpp"
  "${XR3DA_DIR}/Stats.cpp"
  "${XR3DA_DIR}/HW.cpp"
  "${XR3DA_DIR}/HWCaps.cpp"
  "${XR3DA_DIR}/xr_effgamma.cpp"
  "${XR3DA_DIR}/Blenders/Blender.cpp"
  "${XR3DA_DIR}/Blenders/Blender_Palette.cpp"
  "${XR3DA_DIR}/Blenders/Blender_Recorder.cpp"
  "${XR3DA_DIR}/Blender_Recorder_R2.cpp"
  "${XR3DA_DIR}/Blender_Recorder_StandartBinding.cpp"
  "${XR3DA_DIR}/tss_def.cpp"
  "${XR3DA_DIR}/xr_avi.cpp"
  "${XR3DA_DIR}/xrImage_Resampler.cpp"
  "${XR3DA_DIR}/ResourceManager.cpp"
  "${XR3DA_DIR}/ResourceManager_Loader.cpp"
  "${XR3DA_DIR}/ResourceManager_Reset.cpp"
  "${XR3DA_DIR}/ResourceManager_Resources.cpp"
  "${XR3DA_DIR}/ResourceManager_Scripting.cpp"
  "${XR3DA_DIR}/r_constants.cpp"
  "${XR3DA_DIR}/SH_Atomic.cpp"
  "${XR3DA_DIR}/SH_Constant.cpp"
  "${XR3DA_DIR}/SH_Matrix.cpp"
  "${XR3DA_DIR}/SH_RT.cpp"
  "${XR3DA_DIR}/SH_Texture.cpp"
  "${XR3DA_DIR}/Shader.cpp"
  "${XR3DA_DIR}/r_constants_cache.cpp"
  "${XR3DA_DIR}/R_Backend.cpp"
  "${XR3DA_DIR}/R_Backend_DBG.cpp"
  "${XR3DA_DIR}/R_Backend_Runtime.cpp"
  "${XR3DA_DIR}/R_Backend_xform.cpp"
  "${XR3DA_DIR}/R_DStreams.cpp"
  "${XR3DA_DIR}/pure.cpp"
  "${XR3DA_DIR}/xrXRC.cpp"
  "${XR3DA_DIR}/xr_ioc_cmd.cpp"
  "${XR3DA_DIR}/XR_IOConsole.cpp"
  "${XR3DA_DIR}/Engine.cpp"
  "${XR3DA_DIR}/EngineAPI.cpp"
  "${XR3DA_DIR}/EventAPI.cpp"
  "${XR3DA_DIR}/mailSlot.cpp"
  "${XR3DA_DIR}/tga.cpp"
  "${XR3DA_DIR}/_scripting.cpp"
  "${XR3DA_DIR}/ai_script_lua_debug.cpp"
  "${XR3DA_DIR}/ai_script_lua_extension.cpp"
  "${XR3DA_DIR}/class.cpp"
  "${XR3DA_DIR}/class_info.cpp"
  "${XR3DA_DIR}/class_registry.cpp"
  "${XR3DA_DIR}/class_rep.cpp"
  "${XR3DA_DIR}/create_class.cpp"
  "${XR3DA_DIR}/error.cpp"
  "${XR3DA_DIR}/find_best_match.cpp"
  "${XR3DA_DIR}/function.cpp"
  "${XR3DA_DIR}/implicit_cast.cpp"
  "${XR3DA_DIR}/link_compatibility.cpp"
  "${XR3DA_DIR}/object_rep.cpp"
  "${XR3DA_DIR}/open.cpp"
  "${XR3DA_DIR}/pcall.cpp"
  "${XR3DA_DIR}/ref.cpp"
  "${XR3DA_DIR}/scope.cpp"
  "${XR3DA_DIR}/stack_content_by_name.cpp"
  "${XR3DA_DIR}/weak_ref.cpp"
  ${XRSE_FACTORY_LUABIND_OVERLOAD_REP_SOURCE}
  # Legacy d3dx9.lib references the old MSVC _snprintf symbol; keep the
  # compatibility entry point local to XR_3DA.
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc/d3dx9_snprintf_compat.cpp"
)

set(XR3DA_LUABIND_SOURCES
  "${XR3DA_DIR}/class.cpp"
  "${XR3DA_DIR}/class_info.cpp"
  "${XR3DA_DIR}/class_registry.cpp"
  "${XR3DA_DIR}/class_rep.cpp"
  "${XR3DA_DIR}/create_class.cpp"
  "${XR3DA_DIR}/error.cpp"
  "${XR3DA_DIR}/find_best_match.cpp"
  "${XR3DA_DIR}/function.cpp"
  "${XR3DA_DIR}/implicit_cast.cpp"
  "${XR3DA_DIR}/link_compatibility.cpp"
  "${XR3DA_DIR}/object_rep.cpp"
  "${XR3DA_DIR}/open.cpp"
  "${XR3DA_DIR}/pcall.cpp"
  "${XR3DA_DIR}/ref.cpp"
  "${XR3DA_DIR}/scope.cpp"
  "${XR3DA_DIR}/stack_content_by_name.cpp"
  "${XR3DA_DIR}/weak_ref.cpp"
  ${XRSE_FACTORY_LUABIND_OVERLOAD_REP_SOURCE}
)

add_executable(XR_3DA ${XR3DA_SOURCES})

set_target_properties(XR_3DA PROPERTIES
  OUTPUT_NAME "XR_3DA"
  # xrCPU_Pipe links the executable import library from the original
  # x:\xr_3da.lib dependency.
  ENABLE_EXPORTS ON
  # XR_3DA.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
  VS_GLOBAL_CharacterSet "MultiByte"
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
  set_target_properties(XR_3DA PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(XR_3DA
  PRIVATE
    "${XR3DA_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc"
)

if(XRAY_EFFECTIVE_DXSDK_ROOT)
  target_include_directories(XR_3DA PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Include" "${XRAY_EFFECTIVE_DXSDK_ROOT}/include")
  target_link_directories(XR_3DA PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib/x86" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib/x86")
endif()

if(XRSE_FACTORY_LUABIND_INCLUDE_DIR)
  target_include_directories(XR_3DA PRIVATE "${XRSE_FACTORY_LUABIND_INCLUDE_DIR}")
endif()

if(XRAY_BOOST_ROOT)
  target_include_directories(XR_3DA PRIVATE "${XRAY_BOOST_ROOT}")
endif()

target_compile_definitions(XR_3DA
  PRIVATE
    WIN32
    _WINDOWS
    ENGINE_BUILD
    # XR_3DA.vcproj sets ExceptionHandling="FALSE"; keep luabind in the same
    # no-exceptions mode used by the other restored script-binding target.
    LUABIND_NO_EXCEPTIONS
    # XR_3DA Debug.vcproj also defines SECURE_GAMEDATA.
    $<$<CONFIG:Debug>:SECURE_GAMEDATA>
    # XR_3DA includes xrCore.h, so it needs the same modern MSVC compatibility
    # switches for legacy xrCore/STL headers even though they were implicit in
    # the original VC2003 environment.
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Debug>:DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<CONFIG:Mixed>:DEBUG>
    $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(XR_3DA
    PRIVATE
      /nologo
      /W4
      /GR
      /Gz
      /EHs-
      /EHa-
      /Zm200
  )

  # Mirrors XR_3DA.vcproj optimization/runtime choices where modern MSVC still
  # supports them; obsolete VC2003 settings such as /G7 and /Wp64 are omitted.
  target_compile_options(XR_3DA PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Gy /Oi->
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy>
  )

  target_link_options(XR_3DA PRIVATE
    /nologo
    /SUBSYSTEM:WINDOWS
    /MACHINE:I386
    # Legacy DirectX SDK dinput/d3dx9 libraries are not SAFESEH-compatible.
    # d3dx9.lib also carries a VS2003-era /DEFAULTLIB:libcp directive; keep
    # the modern CRT selected by XR_3DA.vcproj's RuntimeLibrary setting.
    /SAFESEH:NO
    /NODEFAULTLIB:libcp
    # stdafx.h, psystem.h, and NET_Shared.h have original hardcoded X: drive
    # pragma libraries. CMake links the already-ported targets directly.
    /NODEFAULTLIB:x:\\xrCore.lib
    /NODEFAULTLIB:x:\\xrCDB.lib
    /NODEFAULTLIB:x:\\xrSound.lib
    /NODEFAULTLIB:x:\\xrLUA.lib
    /NODEFAULTLIB:x:\\xrParticles.lib
    /NODEFAULTLIB:x:\\xrParticlesB.lib
    /NODEFAULTLIB:x:\\xrNetServer.lib
    # Debug renderer/game paths import these helpers from the original
    # xr_3da.lib surface. Keep decorated names in a .def so CMake does not
    # escape '$' inside template-based MSVC symbols.
    "$<$<OR:$<CONFIG:Debug>,$<CONFIG:Mixed>>:/DEF:${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc/xr3da_debug_exports.def>"
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL /MAP>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(XR_3DA PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XR3DA_DIR}/stdafx.h>")
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc/d3dx9_snprintf_compat.cpp" PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
set_source_files_properties(${XR3DA_LUABIND_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
if(MSVC)
  set_source_files_properties(${XR3DA_LUABIND_SOURCES} PROPERTIES
    COMPILE_OPTIONS "/FI${XRSE_FACTORY_DIR}/luabind_compat/xray_luabind_compat.hpp"
  )
endif()

target_link_libraries(XR_3DA PRIVATE
  xrCore
  xrCDB
  xrLUA
  xrParticles
  xrSound
  msacm32
  vfw32
  winmm
  d3d9
  dinput
  dxguid
  d3dx9
)

set(XRRENDER_R1_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xr_3da/xrRender_R1")

if(NOT XRAY_EFFECTIVE_DXSDK_ROOT)
  message(WARNING "xrRender_R1 includes legacy DirectX 9 headers such as d3dx9.h through its PCH. Set XRAY_DXSDK_ROOT, D3DXSDK_DIR, DXSDK_DIR, or DIRECTX_SDK_DIR to the original DirectX SDK root if the active SDK does not provide them.")
elseif(NOT EXISTS "${XRAY_EFFECTIVE_DXSDK_ROOT}/Include/d3dx9.h" AND NOT EXISTS "${XRAY_EFFECTIVE_DXSDK_ROOT}/include/d3dx9.h")
  message(WARNING "xrRender_R1 needs d3dx9.h, but it was not found under XRAY_EFFECTIVE_DXSDK_ROOT='${XRAY_EFFECTIVE_DXSDK_ROOT}'.")
endif()

set(XRRENDER_R1_SOURCES
  "${XRRENDER_R1_DIR}/stdafx.cpp"
  "${XRRENDER_R1_DIR}/xrRender_console.cpp"
  "${XRRENDER_R1_DIR}/xrRender_R1.cpp"
  "${XRRENDER_R1_DIR}/FStaticRender.cpp"
  "${XRRENDER_R1_DIR}/FStaticRender_Blenders.cpp"
  "${XRRENDER_R1_DIR}/FStaticRender_DetectSector.cpp"
  "${XRRENDER_R1_DIR}/FStaticRender_Loader.cpp"
  "${XRRENDER_R1_DIR}/FStaticRender_RenderTarget.cpp"
  "${XRRENDER_R1_DIR}/FStaticRender_Screenshot.cpp"
  "${XRRENDER_R1_DIR}/PSLibrary.cpp"
  "${XRRENDER_R1_DIR}/r__dsgraph_build.cpp"
  "${XRRENDER_R1_DIR}/r__dsgraph_render.cpp"
  "${XRRENDER_R1_DIR}/r__dsgraph_render_lods.cpp"
  "${XRRENDER_R1_DIR}/Texture.cpp"
  "${XRRENDER_R1_DIR}/DetailManager.cpp"
  "${XRRENDER_R1_DIR}/DetailManager_CACHE.cpp"
  "${XRRENDER_R1_DIR}/DetailManager_Decompress.cpp"
  "${XRRENDER_R1_DIR}/DetailManager_soft.cpp"
  "${XRRENDER_R1_DIR}/DetailManager_VS.cpp"
  "${XRRENDER_R1_DIR}/DetailModel.cpp"
  "${XRRENDER_R1_DIR}/NvTriStrip.cpp"
  "${XRRENDER_R1_DIR}/NvTriStripObjects.cpp"
  "${XRRENDER_R1_DIR}/VertexCache.cpp"
  "${XRRENDER_R1_DIR}/xrStripify.cpp"
  "${XRRENDER_R1_DIR}/GlowManager.cpp"
  "${XRRENDER_R1_DIR}/WallmarksEngine.cpp"
  "${XRRENDER_R1_DIR}/Blender_Blur.cpp"
  "${XRRENDER_R1_DIR}/Blender_BmmD.cpp"
  "${XRRENDER_R1_DIR}/Blender_default_aref.cpp"
  "${XRRENDER_R1_DIR}/Blender_detail_still.cpp"
  "${XRRENDER_R1_DIR}/Blender_Editor_Selection.cpp"
  "${XRRENDER_R1_DIR}/Blender_Editor_Wire.cpp"
  "${XRRENDER_R1_DIR}/Blender_LaEmB.cpp"
  "${XRRENDER_R1_DIR}/Blender_Lm(EbB).cpp"
  "${XRRENDER_R1_DIR}/Blender_Model.cpp"
  "${XRRENDER_R1_DIR}/Blender_Model_EbB.cpp"
  "${XRRENDER_R1_DIR}/Blender_Particle.cpp"
  "${XRRENDER_R1_DIR}/Blender_Screen_GRAY.cpp"
  "${XRRENDER_R1_DIR}/Blender_Screen_SET.cpp"
  "${XRRENDER_R1_DIR}/Blender_Shadow_World.cpp"
  "${XRRENDER_R1_DIR}/Blender_tree.cpp"
  "${XRRENDER_R1_DIR}/Blender_Vertex.cpp"
  "${XRRENDER_R1_DIR}/Blender_Vertex_aref.cpp"
  "${XRRENDER_R1_DIR}/BlenderDefault.cpp"
  "${XRRENDER_R1_DIR}/ModelPool.cpp"
  "${XRRENDER_R1_DIR}/FTreeVisual.cpp"
  "${XRRENDER_R1_DIR}/ParticleEffect.cpp"
  "${XRRENDER_R1_DIR}/ParticleEffectDef.cpp"
  "${XRRENDER_R1_DIR}/ParticleGroup.cpp"
  "${XRRENDER_R1_DIR}/psvisual.cpp"
  "${XRRENDER_R1_DIR}/r__sector.cpp"
  "${XRRENDER_R1_DIR}/r__sector_traversal.cpp"
  "${XRRENDER_R1_DIR}/HOM.cpp"
  "${XRRENDER_R1_DIR}/occRasterizer.cpp"
  "${XRRENDER_R1_DIR}/occRasterizer_core.cpp"
  "${XRRENDER_R1_DIR}/light.cpp"
  "${XRRENDER_R1_DIR}/Light_DB.cpp"
  "${XRRENDER_R1_DIR}/Light_Package.cpp"
  "${XRRENDER_R1_DIR}/LightPPA.cpp"
  "${XRRENDER_R1_DIR}/LightProjector.cpp"
  # LightProjector_Cache.cpp is present in xrRender_R1.vcproj but excluded
  # from all original configurations.
  "${XRRENDER_R1_DIR}/LightShadows.cpp"
  "${XRRENDER_R1_DIR}/LightTrack.cpp"
  # Legacy d3dx9.lib references old MSVC CRT symbols; keep the compatibility
  # entry points local to the renderer that links those legacy D3DX helpers.
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc/d3dx9_r1_crt_compat.cpp"
)

add_library(xrRender_R1 SHARED ${XRRENDER_R1_SOURCES})

set_target_properties(xrRender_R1 PROPERTIES
  OUTPUT_NAME "xrRender_R1"
  # xrRender_R1.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${XRAY_LIBRARY_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(xrRender_R1 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrRender_R1
  PRIVATE
    "${XRRENDER_R1_DIR}"
    "${XR3DA_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/xrCore"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc"
)

if(XRAY_EFFECTIVE_DXSDK_ROOT)
  target_include_directories(xrRender_R1 PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Include" "${XRAY_EFFECTIVE_DXSDK_ROOT}/include")
  target_link_directories(xrRender_R1 PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib/x86" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib/x86")
endif()

target_compile_definitions(xrRender_R1
  PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    XRRENDER_R1_EXPORTS
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
    $<$<CONFIG:Debug>:_DEBUG;DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xrRender_R1
    PRIVATE
      /nologo
      /W4
      /GR
      /Gz
      /EHs-
      /EHa-
  )

  # Mirrors xrRender_R1.vcproj optimization/runtime choices where modern MSVC
  # still supports them; obsolete VC2003 settings such as /G7 and /Wp64 are
  # omitted.
  target_compile_options(xrRender_R1 PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Gy /Oi->
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy>
  )

  target_link_options(xrRender_R1 PRIVATE
    /nologo
    /SUBSYSTEM:WINDOWS
    /MACHINE:I386
    # xrRender_R1 includes xr_3da/stdafx.h and xrRender_R1.cpp carries an
    # x:\xr_3DA pragma. CMake links the already-ported targets directly.
    /NODEFAULTLIB:x:\\xrCore.lib
    /NODEFAULTLIB:x:\\xrCDB.lib
    /NODEFAULTLIB:x:\\xrSound.lib
    /NODEFAULTLIB:x:\\xrLUA.lib
    /NODEFAULTLIB:x:\\xrParticles.lib
    /NODEFAULTLIB:x:\\xr_3DA.lib
    # Legacy DirectX SDK d3dx9.lib is not SAFESEH-compatible and carries a
    # VS2003-era /DEFAULTLIB:libcp directive.
    /SAFESEH:NO
    /NODEFAULTLIB:libcp
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL /MAP>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrRender_R1 PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XRRENDER_R1_DIR}/stdafx.h>")
set_source_files_properties("${XRRENDER_R1_DIR}/stdafx.cpp" PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc/d3dx9_r1_crt_compat.cpp" PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

target_link_libraries(xrRender_R1 PRIVATE
  XR_3DA
  xrCore
  xrCDB
  xrSound
  xrLUA
  xrParticles
  winmm
  d3d9
  dinput
  dxguid
  d3dx9
)

set(XRRENDER_R2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xr_3da/xrRender_R2")

if(NOT XRAY_EFFECTIVE_DXSDK_ROOT)
  message(WARNING "xrRender_R2 includes legacy DirectX 9 headers such as d3dx9.h through its PCH. Set XRAY_DXSDK_ROOT, D3DXSDK_DIR, DXSDK_DIR, or DIRECTX_SDK_DIR to the original DirectX SDK root if the active SDK does not provide them.")
elseif(NOT EXISTS "${XRAY_EFFECTIVE_DXSDK_ROOT}/Include/d3dx9.h" AND NOT EXISTS "${XRAY_EFFECTIVE_DXSDK_ROOT}/include/d3dx9.h")
  message(WARNING "xrRender_R2 needs d3dx9.h, but it was not found under XRAY_EFFECTIVE_DXSDK_ROOT='${XRAY_EFFECTIVE_DXSDK_ROOT}'.")
endif()

set(XRRENDER_R2_SOURCES
  "${XRRENDER_R2_DIR}/stdafx.cpp"
  "${XRRENDER_R2_DIR}/xrRender_console.cpp"
  "${XRRENDER_R2_DIR}/xrRender_R2.cpp"
  "${XRRENDER_R2_DIR}/PSLibrary.cpp"
  "${XRRENDER_R2_DIR}/r2.cpp"
  "${XRRENDER_R2_DIR}/r2_blenders.cpp"
  "${XRRENDER_R2_DIR}/r2_loader.cpp"
  "${XRRENDER_R2_DIR}/r2_R_calculate.cpp"
  "${XRRENDER_R2_DIR}/r2_R_lights.cpp"
  "${XRRENDER_R2_DIR}/r2_R_render.cpp"
  "${XRRENDER_R2_DIR}/r2_R_sun.cpp"
  "${XRRENDER_R2_DIR}/r2_screenshot.cpp"
  "${XRRENDER_R2_DIR}/r2_sector_detect.cpp"
  "${XRRENDER_R2_DIR}/r__dsgraph_build.cpp"
  "${XRRENDER_R2_DIR}/r__dsgraph_render.cpp"
  "${XRRENDER_R2_DIR}/r__dsgraph_render_lods.cpp"
  "${XRRENDER_R2_DIR}/r__occlusion.cpp"
  "${XRRENDER_R2_DIR}/Texture.cpp"
  "${XRRENDER_R2_DIR}/DetailManager.cpp"
  "${XRRENDER_R2_DIR}/DetailManager_CACHE.cpp"
  "${XRRENDER_R2_DIR}/DetailManager_Decompress.cpp"
  "${XRRENDER_R2_DIR}/DetailManager_soft.cpp"
  "${XRRENDER_R2_DIR}/DetailManager_VS.cpp"
  "${XRRENDER_R2_DIR}/DetailModel.cpp"
  "${XRRENDER_R2_DIR}/NvTriStrip.cpp"
  "${XRRENDER_R2_DIR}/NvTriStripObjects.cpp"
  "${XRRENDER_R2_DIR}/VertexCache.cpp"
  "${XRRENDER_R2_DIR}/xrStripify.cpp"
  "${XRRENDER_R2_DIR}/light.cpp"
  "${XRRENDER_R2_DIR}/Light_DB.cpp"
  "${XRRENDER_R2_DIR}/light_GI.cpp"
  "${XRRENDER_R2_DIR}/Light_Package.cpp"
  "${XRRENDER_R2_DIR}/Light_Render_Direct.cpp"
  "${XRRENDER_R2_DIR}/Light_Render_Direct_ComputeXFS.cpp"
  "${XRRENDER_R2_DIR}/light_smapvis.cpp"
  "${XRRENDER_R2_DIR}/light_vis.cpp"
  "${XRRENDER_R2_DIR}/WallmarksEngine.cpp"
  "${XRRENDER_R2_DIR}/blender_bloom_build.cpp"
  "${XRRENDER_R2_DIR}/blender_combine.cpp"
  "${XRRENDER_R2_DIR}/blender_deffer_aref.cpp"
  "${XRRENDER_R2_DIR}/blender_deffer_flat.cpp"
  "${XRRENDER_R2_DIR}/blender_deffer_model.cpp"
  "${XRRENDER_R2_DIR}/blender_light_direct.cpp"
  "${XRRENDER_R2_DIR}/blender_light_mask.cpp"
  "${XRRENDER_R2_DIR}/blender_light_occq.cpp"
  "${XRRENDER_R2_DIR}/blender_light_point.cpp"
  "${XRRENDER_R2_DIR}/blender_light_reflected.cpp"
  "${XRRENDER_R2_DIR}/blender_light_spot.cpp"
  "${XRRENDER_R2_DIR}/blender_luminance.cpp"
  "${XRRENDER_R2_DIR}/Blender_Particle.cpp"
  "${XRRENDER_R2_DIR}/uber_deffer.cpp"
  "${XRRENDER_R2_DIR}/Blender_detail_still.cpp"
  "${XRRENDER_R2_DIR}/Blender_Screen_SET.cpp"
  "${XRRENDER_R2_DIR}/Blender_tree.cpp"
  "${XRRENDER_R2_DIR}/ModelPool.cpp"
  "${XRRENDER_R2_DIR}/FTreeVisual.cpp"
  "${XRRENDER_R2_DIR}/ParticleEffect.cpp"
  "${XRRENDER_R2_DIR}/ParticleEffectDef.cpp"
  "${XRRENDER_R2_DIR}/ParticleGroup.cpp"
  "${XRRENDER_R2_DIR}/psvisual.cpp"
  "${XRRENDER_R2_DIR}/r__sector.cpp"
  "${XRRENDER_R2_DIR}/r__sector_traversal.cpp"
  "${XRRENDER_R2_DIR}/HOM.cpp"
  "${XRRENDER_R2_DIR}/occRasterizer.cpp"
  "${XRRENDER_R2_DIR}/occRasterizer_core.cpp"
  "${XRRENDER_R2_DIR}/du_sphere_part.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_accum_direct.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_accum_omnipart_geom.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_accum_point.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_accum_point_geom.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_accum_reflected.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_accum_spot.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_accum_spot_geom.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_draw_volume.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_enable_scissor.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_phase_accumulator.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_phase_accumulator_init.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_phase_bloom.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_phase_combine.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_phase_distortion.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_phase_luminance.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_phase_occq.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_phase_scene.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_phase_smap_D.cpp"
  "${XRRENDER_R2_DIR}/r2_rendertarget_phase_smap_S.cpp"
  # Legacy d3dx9.lib references old MSVC CRT symbols; keep the compatibility
  # entry points local to the renderer that links those legacy D3DX helpers.
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc/d3dx9_r2_crt_compat.cpp"
)

add_library(xrRender_R2 SHARED ${XRRENDER_R2_SOURCES})

set_target_properties(xrRender_R2 PROPERTIES
  OUTPUT_NAME "xrRender_R2"
  # xrRender_R2.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${XRAY_LIBRARY_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(xrRender_R2 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrRender_R2
  PRIVATE
    "${XRRENDER_R2_DIR}"
    "${XR3DA_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/xrCore"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc"
)

if(XRAY_EFFECTIVE_DXSDK_ROOT)
  target_include_directories(xrRender_R2 PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Include" "${XRAY_EFFECTIVE_DXSDK_ROOT}/include")
  target_link_directories(xrRender_R2 PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib/x86" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib/x86")
endif()

target_compile_definitions(xrRender_R2
  PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    XRRENDER_R2_EXPORTS
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
    $<$<CONFIG:Debug>:_DEBUG;DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xrRender_R2
    PRIVATE
      /nologo
      /W4
      /GR
      /Gz
      /EHs-
      /EHa-
  )

  # Mirrors xrRender_R2.vcproj optimization/runtime choices where modern MSVC
  # still supports them; obsolete VC2003 settings such as /G7 and /Wp64 are
  # omitted.
  target_compile_options(xrRender_R2 PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Gy /Oi->
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy>
  )

  target_link_options(xrRender_R2 PRIVATE
    /nologo
    /SUBSYSTEM:WINDOWS
    /MACHINE:I386
    # xrRender_R2 includes xr_3da/stdafx.h and xrRender_R2.cpp carries an
    # x:\xr_3DA pragma. CMake links the already-ported targets directly.
    /NODEFAULTLIB:x:\\xrCore.lib
    /NODEFAULTLIB:x:\\xrCDB.lib
    /NODEFAULTLIB:x:\\xrSound.lib
    /NODEFAULTLIB:x:\\xrLUA.lib
    /NODEFAULTLIB:x:\\xrParticles.lib
    /NODEFAULTLIB:x:\\xr_3DA.lib
    # Legacy DirectX SDK d3dx9.lib is not SAFESEH-compatible and carries a
    # VS2003-era /DEFAULTLIB:libcp directive.
    /SAFESEH:NO
    /NODEFAULTLIB:libcp
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL /MAP>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrRender_R2 PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XRRENDER_R2_DIR}/stdafx.h>")
set_source_files_properties("${XRRENDER_R2_DIR}/stdafx.cpp" PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc/d3dx9_r2_crt_compat.cpp" PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

target_link_libraries(xrRender_R2 PRIVATE
  XR_3DA
  xrCore
  xrCDB
  xrSound
  xrLUA
  xrParticles
  winmm
  d3d9
  dinput
  dxguid
  d3dx9
)

set(XRCPU_PIPE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xr_3da/xrCPU_Pipe")

if(NOT XRAY_EFFECTIVE_DXSDK_ROOT)
  message(WARNING "xrCPU_Pipe uses xr_3da/stdafx.h, which includes legacy DirectPlay header dplay8.h. Set XRAY_DXSDK_ROOT, D3DXSDK_DIR, DXSDK_DIR, or DIRECTX_SDK_DIR to the original DirectX SDK root if the active SDK does not provide it.")
elseif(NOT EXISTS "${XRAY_EFFECTIVE_DXSDK_ROOT}/Include/dplay8.h" AND NOT EXISTS "${XRAY_EFFECTIVE_DXSDK_ROOT}/include/dplay8.h")
  message(WARNING "xrCPU_Pipe needs dplay8.h through xr_3da/stdafx.h, but it was not found under XRAY_EFFECTIVE_DXSDK_ROOT='${XRAY_EFFECTIVE_DXSDK_ROOT}'.")
endif()

set(XRCPU_PIPE_SOURCES
  "${XRCPU_PIPE_DIR}/StdAfx.cpp"
  "${XRCPU_PIPE_DIR}/xrCPU_Pipe.cpp"
  "${XRCPU_PIPE_DIR}/3DNow!Lib.cpp"
  "${XRCPU_PIPE_DIR}/xrM44mul_3DNow!.cpp"
  "${XRCPU_PIPE_DIR}/xrMemCopy8_3DNow!.cpp"
  "${XRCPU_PIPE_DIR}/xrMemFill32_3DNow!.cpp"
  "${XRCPU_PIPE_DIR}/xrM44mul.cpp"
  "${XRCPU_PIPE_DIR}/xrQuatSlerp.cpp"
  "${XRCPU_PIPE_DIR}/xrSkin1W.cpp"
  "${XRCPU_PIPE_DIR}/xrSkin2W.cpp"
  "${XRCPU_PIPE_DIR}/xrTransfer.cpp"
)

add_library(xrCPU_Pipe SHARED ${XRCPU_PIPE_SOURCES})

set_target_properties(xrCPU_Pipe PROPERTIES
  OUTPUT_NAME "xrCPU_Pipe"
  # xrCPU_Pipe.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
  VS_GLOBAL_CharacterSet "MultiByte"
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
  set_target_properties(xrCPU_Pipe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrCPU_Pipe
  PRIVATE
    "${XRCPU_PIPE_DIR}"
    "${XR3DA_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/xrCore"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc"
)

if(XRAY_EFFECTIVE_DXSDK_ROOT)
  target_include_directories(xrCPU_Pipe PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Include" "${XRAY_EFFECTIVE_DXSDK_ROOT}/include")
  target_link_directories(xrCPU_Pipe PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib/x86" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib/x86")
endif()

target_compile_definitions(xrCPU_Pipe
  PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    XRCPU_PIPE_EXPORTS
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
    $<$<CONFIG:Debug>:_DEBUG;DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xrCPU_Pipe
    PRIVATE
      /nologo
      /W4
      /GR
      /Gz
      /EHs-
      /EHa-
  )

  # Mirrors xrCPU_Pipe.vcproj optimization/runtime choices where modern MSVC
  # still supports them; obsolete VC2003 settings such as /G7 and /Wp64 are
  # omitted.
  target_compile_options(xrCPU_Pipe PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Gy /Oi->
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy>
  )

  target_link_options(xrCPU_Pipe PRIVATE
    /nologo
    /SUBSYSTEM:WINDOWS
    /MACHINE:I386
    # xrCPU_Pipe.vcproj linked x:\xr_3da.lib. CMake links the XR_3DA
    # executable import library target directly instead.
    /NODEFAULTLIB:x:\\xr_3da.lib
    # xr_3da/stdafx.h carries the historical hardcoded pragma, while this
    # target links the CMake xrCore import library explicitly below.
    /NODEFAULTLIB:x:\\xrCore.lib
    # Same historical pragma path from xr_3da/stdafx.h; link xrCDB through
    # the CMake target instead of the old X: drive import library.
    /NODEFAULTLIB:x:\\xrCDB.lib
    # Same historical pragma path from xr_3da/stdafx.h; link xrSound through
    # the CMake target instead of the old X: drive import library.
    /NODEFAULTLIB:x:\\xrSound.lib
    # Same historical pragma path from xr_3da/stdafx.h; link xrLUA through
    # the CMake target instead of the old X: drive import library.
    /NODEFAULTLIB:x:\\xrLUA.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL /MAP>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrCPU_Pipe PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XRCPU_PIPE_DIR}/StdAfx.h>")
set_source_files_properties("${XRCPU_PIPE_DIR}/StdAfx.cpp" PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

target_link_libraries(xrCPU_Pipe PRIVATE
  XR_3DA
  xrCore
  xrCDB
  xrSound
  xrLUA
  odbc32
  odbccp32
)

set(XRGAME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xr_3da/xrGame")
set(XRGAME_VCPROJ "${XRGAME_DIR}/xrGame.vcproj")

find_program(XRAY_POWERSHELL_EXECUTABLE powershell REQUIRED)

execute_process(
  COMMAND "${XRAY_POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
          -File "${CMAKE_CURRENT_SOURCE_DIR}/cmake/read_vs2003_sources.ps1"
          "${XRGAME_VCPROJ}" "${XRGAME_DIR}" -Mode sources
  OUTPUT_VARIABLE XRGAME_SOURCES_RAW
  RESULT_VARIABLE XRGAME_SOURCES_RESULT
  ERROR_VARIABLE XRGAME_SOURCES_ERROR
)
if(NOT XRGAME_SOURCES_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to read xrGame sources from ${XRGAME_VCPROJ}: ${XRGAME_SOURCES_ERROR}")
endif()
if(NOT XRGAME_SOURCES_RAW)
  message(FATAL_ERROR "No xrGame sources were read from ${XRGAME_VCPROJ}")
endif()
set(XRGAME_SOURCES "${XRGAME_SOURCES_RAW}")
set(XRGAME_LUABIND_OBJECT_SOURCE "${XRGAME_DIR}/luabind/object.cpp")
# xrGame.vcproj lists this local object/proxy implementation, but the matching
# GSC-era luabind public headers are missing. The active luabind 0.7 headers
# provide object/proxy support through their public headers; XR_3DA and
# xrSE_Factory already omit this obsolete source and compile the remaining
# local luabind sources with the compatibility overlay.
list(REMOVE_ITEM XRGAME_SOURCES "${XRGAME_LUABIND_OBJECT_SOURCE}")

# Graph-format spawn loading in xrGame owns CServerEntityWrapper at runtime.
# The wrapper implementation mirrors xrAI's helper, but lives in xrGame for
# the restored runtime-side spawn registry and is not part of the old vcproj.
list(APPEND XRGAME_SOURCES "${XRGAME_DIR}/server_entity_wrapper.cpp")

# Local luabind class_rep.cpp calls luabind::detail::overload_rep::call; use
# the same luabind 0.7 implementation source already required by XR_3DA and
# xrSE_Factory, without resurrecting the obsolete local object.cpp.
if(XRSE_FACTORY_LUABIND_OVERLOAD_REP_SOURCE)
  list(APPEND XRGAME_SOURCES "${XRSE_FACTORY_LUABIND_OVERLOAD_REP_SOURCE}")
endif()
list(LENGTH XRGAME_SOURCES XRGAME_SOURCE_COUNT)
message(STATUS "xrGame sources imported from xrGame.vcproj: ${XRGAME_SOURCE_COUNT}")

execute_process(
  COMMAND "${XRAY_POWERSHELL_EXECUTABLE}" -NoProfile -ExecutionPolicy Bypass
          -File "${CMAKE_CURRENT_SOURCE_DIR}/cmake/read_vs2003_sources.ps1"
          "${XRGAME_VCPROJ}" "${XRGAME_DIR}" -Mode no-pch -Configuration "Debug|Win32"
  OUTPUT_VARIABLE XRGAME_NO_PCH_SOURCES_RAW
  RESULT_VARIABLE XRGAME_NO_PCH_RESULT
  ERROR_VARIABLE XRGAME_NO_PCH_ERROR
)
if(NOT XRGAME_NO_PCH_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to read xrGame no-PCH sources from ${XRGAME_VCPROJ}: ${XRGAME_NO_PCH_ERROR}")
endif()
set(XRGAME_NO_PCH_SOURCES "${XRGAME_NO_PCH_SOURCES_RAW}")

set(XRGAME_LUABIND_SOURCES ${XRGAME_SOURCES})
list(FILTER XRGAME_LUABIND_SOURCES INCLUDE REGEX "/luabind/")

if(NOT XRAY_EFFECTIVE_DXSDK_ROOT)
  message(WARNING "xrGame uses xr_3da/stdafx.h and dinput.h, and transitively needs legacy DirectX SDK headers such as dplay8.h. Set XRAY_DXSDK_ROOT, D3DXSDK_DIR, DXSDK_DIR, or DIRECTX_SDK_DIR to the original DirectX SDK root if the active SDK does not provide them.")
elseif(NOT EXISTS "${XRAY_EFFECTIVE_DXSDK_ROOT}/Include/dplay8.h" AND NOT EXISTS "${XRAY_EFFECTIVE_DXSDK_ROOT}/include/dplay8.h")
  message(WARNING "xrGame needs dplay8.h through xr_3da/stdafx.h, but it was not found under XRAY_EFFECTIVE_DXSDK_ROOT='${XRAY_EFFECTIVE_DXSDK_ROOT}'.")
endif()

add_library(xrGame SHARED ${XRGAME_SOURCES})

set_target_properties(xrGame PROPERTIES
  OUTPUT_NAME "xrGame"
  # xrGame.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
  VS_GLOBAL_CharacterSet "MultiByte"
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
  set_target_properties(xrGame PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrGame
  PRIVATE
    "${XRGAME_DIR}"
    "${XRGAME_DIR}/ui"
    "${XRGAME_DIR}/GameSpy"
    "${XRGAME_DIR}/GameSpy/Qr2"
    "${XRGAME_DIR}/GameSpy/CDKey"
    "${XR3DA_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/xrCore"
    "${CMAKE_CURRENT_SOURCE_DIR}/xrLUA/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/xrLUA/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc"
)

if(XRAY_EFFECTIVE_DXSDK_ROOT)
  target_include_directories(xrGame PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Include" "${XRAY_EFFECTIVE_DXSDK_ROOT}/include")
  target_link_directories(xrGame PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib/x86" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib/x86")
endif()

if(XRSE_FACTORY_LUABIND_INCLUDE_DIR)
  target_include_directories(xrGame PRIVATE "${XRSE_FACTORY_LUABIND_INCLUDE_DIR}")
endif()

if(XRAY_BOOST_ROOT)
  target_include_directories(xrGame PRIVATE "${XRAY_BOOST_ROOT}")
endif()

if(XRAY_LOKI_ROOT)
  target_include_directories(xrGame PRIVATE "${XRAY_LOKI_ROOT}")
endif()

target_compile_definitions(xrGame
  PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    XRGAME_EXPORTS
    dSINGLE
    MSVC
    LUABIND_NO_EXCEPTIONS
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
    $<$<CONFIG:Debug>:_DEBUG;DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<CONFIG:Mixed>:DEBUG>
    $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xrGame
    PRIVATE
      /nologo
      /W4
      /GR
      /Gz
      /EHs-
      /EHa-
      /Zm200
  )

  # Mirrors xrGame.vcproj optimization/runtime choices where modern MSVC still
  # supports them; obsolete VC2003 settings such as /G7 and /Wp64 are omitted.
  target_compile_options(xrGame PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Gy /Oi->
    $<$<CONFIG:Debug>:/FS>
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy>
  )

  target_link_options(xrGame PRIVATE
    /nologo
    /SUBSYSTEM:WINDOWS
    /MACHINE:I386
    # xrGame.vcproj linked x:\xr_3da.lib. CMake links the XR_3DA executable
    # import library target directly instead.
    /NODEFAULTLIB:x:\\xr_3da.lib
    # Historical hardcoded pragma paths from inherited engine headers; link the
    # already-restored CMake targets explicitly below.
    /NODEFAULTLIB:x:\\xrCore.lib
    /NODEFAULTLIB:x:\\xrCDB.lib
    /NODEFAULTLIB:x:\\xrSound.lib
    /NODEFAULTLIB:x:\\xrLUA.lib
    /NODEFAULTLIB:x:\\xrParticles.lib
    /NODEFAULTLIB:x:\\xrParticlesB.lib
    /NODEFAULTLIB:x:\\xrNetServer.lib
    /NODEFAULTLIB:x:\\xrXMLParser.lib
    /NODEFAULTLIB:ode.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL /MAP>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrGame PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XRGAME_DIR}/stdafx.h>")
set_source_files_properties("${XRGAME_DIR}/StdAfx.cpp" PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
if(XRGAME_NO_PCH_SOURCES)
  set_source_files_properties(${XRGAME_NO_PCH_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()
if(XRGAME_LUABIND_SOURCES)
  set_source_files_properties(${XRGAME_LUABIND_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
  if(MSVC)
    set_source_files_properties(${XRGAME_LUABIND_SOURCES} PROPERTIES
      COMPILE_OPTIONS "/FI${XRSE_FACTORY_DIR}/luabind_compat/xray_luabind_compat.hpp"
    )
  endif()
endif()

target_link_libraries(xrGame PRIVATE
  XR_3DA
  xrCore
  xrCDB
  xrLUA
  xrXMLParser
  xrParticles
  xrNetServer
  xrSound
  xrCPU_Pipe
  ode
  winmm
  ws2_32
  wsock32
  advapi32
  d3d9
  dinput
  dxguid
  $<$<NOT:$<CONFIG:Debug>>:odbc32>
  $<$<NOT:$<CONFIG:Debug>>:odbccp32>
)
