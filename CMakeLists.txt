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

set(XRCDB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xr_3da/xrCDB")

set(XRCDB_SOURCES
  "${XRCDB_DIR}/Frustum.cpp"
  "${XRCDB_DIR}/StdAfx.cpp"
  "${XRCDB_DIR}/xrCDB.cpp"
  "${XRCDB_DIR}/xrCDB_box.cpp"
  "${XRCDB_DIR}/xrCDB_Collector.cpp"
  "${XRCDB_DIR}/xrCDB_frustum.cpp"
  "${XRCDB_DIR}/xrCDB_ray.cpp"
  "${XRCDB_DIR}/OPC_AABB.cpp"
  "${XRCDB_DIR}/OPC_AABBCollider.cpp"
  "${XRCDB_DIR}/OPC_AABBTree.cpp"
  "${XRCDB_DIR}/OPC_Collider.cpp"
  "${XRCDB_DIR}/OPC_Common.cpp"
  "${XRCDB_DIR}/OPC_Container.cpp"
  "${XRCDB_DIR}/OPC_Matrix3x3.cpp"
  "${XRCDB_DIR}/OPC_Matrix4x4.cpp"
  "${XRCDB_DIR}/OPC_Model.cpp"
  "${XRCDB_DIR}/OPC_OBB.cpp"
  "${XRCDB_DIR}/OPC_OBBCollider.cpp"
  "${XRCDB_DIR}/OPC_OptimizedTree.cpp"
  "${XRCDB_DIR}/OPC_Plane.cpp"
  "${XRCDB_DIR}/OPC_PlanesCollider.cpp"
  "${XRCDB_DIR}/OPC_Point.cpp"
  "${XRCDB_DIR}/OPC_Ray.cpp"
  "${XRCDB_DIR}/OPC_RayCollider.cpp"
  "${XRCDB_DIR}/OPC_SphereCollider.cpp"
  "${XRCDB_DIR}/OPC_TreeBuilders.cpp"
  "${XRCDB_DIR}/OPC_TreeCollider.cpp"
  "${XRCDB_DIR}/OPC_Triangle.cpp"
  "${XRCDB_DIR}/OPC_VolumeCollider.cpp"
  "${XRCDB_DIR}/Opcode.cpp"
)

add_library(xrCDB SHARED ${XRCDB_SOURCES})

set_target_properties(xrCDB PROPERTIES
  OUTPUT_NAME "xrCDB"
  # xrCDB.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
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
  set_target_properties(xrCDB PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

# xrCDB.vcproj has no AdditionalIncludeDirectories. The project directory is
# still public because engine consumers include xrCDB headers directly.
target_include_directories(xrCDB
  PUBLIC
    "${XRCDB_DIR}"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc"
)

target_compile_definitions(xrCDB
  PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    XRCDB_EXPORTS
    # xrCDB includes xrCore.h, so it needs the same modern MSVC compatibility
    # switches for legacy xrCore/STL headers even though they were implicit in
    # the original VC2003 environment.
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xrCDB
    PRIVATE
      /nologo
      /W4
      /GR
      /Gz
      /EHs-
      /EHa-
  )

  # Mirrors xrCDB.vcproj optimization/runtime choices where modern MSVC still
  # supports them; obsolete VC2003 settings such as /G7 and /Wp64 are omitted.
  target_compile_options(xrCDB PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Gy /Oi->
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy /Zc:wchar_t->
  )

  target_link_options(xrCDB PRIVATE
    /nologo
    # StdAfx.cpp has the original #pragma comment(lib, "x:\\xrCore.lib").
    # CMake links the xrCore target directly, so ignore only that hardcoded
    # X: drive default library directive.
    /NODEFAULTLIB:x:\\xrCore.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrCDB PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XRCDB_DIR}/stdafx.h>")

target_link_libraries(xrCDB PRIVATE
  xrCore
  winmm
  $<$<CONFIG:Debug>:odbc32>
  $<$<CONFIG:Debug>:odbccp32>
)

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

set(XRXMLPARSER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xrXMLParser")

set(XRXMLPARSER_SOURCES
  "${XRXMLPARSER_DIR}/stdafx.cpp"
  "${XRXMLPARSER_DIR}/xrXMLParser.cpp"
  "${XRXMLPARSER_DIR}/tinystr.cpp"
  "${XRXMLPARSER_DIR}/tinyxml.cpp"
  "${XRXMLPARSER_DIR}/tinyxmlerror.cpp"
  "${XRXMLPARSER_DIR}/tinyxmlparser.cpp"
)

add_library(xrXMLParser SHARED ${XRXMLPARSER_SOURCES})

set_target_properties(xrXMLParser PROPERTIES
  OUTPUT_NAME "xrXMLParser"
  # xrXMLParser.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:DebugDLL>"
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(xrXMLParser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrXMLParser
  PUBLIC
    "${XRXMLPARSER_DIR}"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc"
)

target_compile_definitions(xrXMLParser
  PRIVATE
    WIN32
    _WINDOWS
    XRXMLPARSER_EXPORTS
    # xrXMLParser includes xrCore.h, so it needs the same modern MSVC
    # compatibility switches for legacy xrCore/STL headers even though they
    # were implicit in the original VC2003 environment.
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
)

if(MSVC)
  target_compile_options(xrXMLParser
    PRIVATE
      /nologo
      /W4
      /GR
      /EHs-
      /EHa-
      /Gd
      /TP
  )

  # Mirrors xrXMLParser.vcproj optimization/runtime choices where modern MSVC
  # still supports them; obsolete VC2003 settings such as /G7 and /Wp64 are
  # omitted.
  target_compile_options(xrXMLParser PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Gy /Oi->
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy /Zc:wchar_t->
  )

  target_link_options(xrXMLParser PRIVATE
    /nologo
    /MACHINE:I386
    # stdafx.h has the original #pragma comment(lib, "x:\\xrCore.lib").
    # CMake links xrCore directly, so ignore only that hardcoded X: drive
    # default library directive.
    /NODEFAULTLIB:x:\\xrCore.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL /MAP>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrXMLParser PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XRXMLPARSER_DIR}/stdafx.h>")

target_link_libraries(xrXMLParser PRIVATE
  xrCore
  odbc32
  odbccp32
  wininet
  $<$<CONFIG:Debug>:${XRXMLPARSER_DIR}/ChilkatDbgDll.lib>
  $<$<CONFIG:Debug>:${XRXMLPARSER_DIR}/CkBaseDbgDll.lib>
  $<$<NOT:$<CONFIG:Debug>>:${XRXMLPARSER_DIR}/ChilkatRelDll.lib>
  $<$<NOT:$<CONFIG:Debug>>:${XRXMLPARSER_DIR}/CkBaseRelDll.lib>
)

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

set(XRPARTICLES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xrParticles")

set(XRPARTICLES_SOURCES
  "${XRPARTICLES_DIR}/stdafx.cpp"
  "${XRPARTICLES_DIR}/noise.cpp"
  "${XRPARTICLES_DIR}/particle_actions.cpp"
  "${XRPARTICLES_DIR}/particle_actions_collection.cpp"
  "${XRPARTICLES_DIR}/particle_actions_collection_io.cpp"
  "${XRPARTICLES_DIR}/particle_core.cpp"
  "${XRPARTICLES_DIR}/particle_effect.cpp"
  "${XRPARTICLES_DIR}/particle_manager.cpp"
)

add_library(xrParticles SHARED ${XRPARTICLES_SOURCES})

set_target_properties(xrParticles PROPERTIES
  OUTPUT_NAME "xrParticles"
  # xrParticles.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
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
  set_target_properties(xrParticles PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrParticles
  PUBLIC
    "${XRPARTICLES_DIR}"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat/msvc"
)

target_compile_definitions(xrParticles
  PRIVATE
    WIN32
    _WINDOWS
    XR_PARTICLES_EXPORTS
    # xrParticles includes xrCore.h, so it needs the same modern MSVC
    # compatibility switches for legacy xrCore/STL headers even though they
    # were implicit in the original VC2003 environment.
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xrParticles
    PRIVATE
      /nologo
      /W4
      /GR
      /Gz
      /EHs-
      /EHa-
  )

  # Mirrors xrParticles.vcproj optimization/runtime choices where modern MSVC
  # still supports them; obsolete VC2003 settings such as /G7 and /Wp64 are
  # omitted.
  target_compile_options(xrParticles PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Oi->
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy>
  )

  target_link_options(xrParticles PRIVATE
    /nologo
    # stdafx.h and psystem.h have original #pragma comment(lib, "x:\\...").
    # CMake links xrCore directly and builds xrParticles here, so ignore only
    # those hardcoded X: drive default library directives.
    /NODEFAULTLIB:x:\\xrCore.lib
    /NODEFAULTLIB:x:\\xrParticles.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrParticles PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XRPARTICLES_DIR}/stdafx.h>")

target_link_libraries(xrParticles PRIVATE
  xrCore
  odbc32
  odbccp32
)

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
