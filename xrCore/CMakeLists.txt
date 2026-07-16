set(XRCORE_SOURCES
  FTimer.cpp
  stdafx.cpp
  xrCore.cpp
  xrCore.rc
  _compressed_normal.cpp
  _math.cpp
  _sphere.cpp
  cpuid.cpp
  _std_extensions.cpp
  clsid.cpp
  xr_trims.cpp
  FileSystem.cpp
  FileSystem_borland.cpp
  FS.cpp
  LocatorAPI.cpp
  LocatorAPI_auth.cpp
  LocatorAPI_defs.cpp
  log.cpp
  Xr_ini.cpp
  xrMemory.cpp
  xrMemory_align.cpp
  xrMemory_debug.cpp
  xrMemory_POOL.cpp
  xrMemory_pso_Copy.cpp
  xrMemory_pso_Fill.cpp
  xrMemory_pso_Fill32.cpp
  xrMemory_subst_borland.cpp
  xrMemory_subst_msvc.cpp
  xrDebug.cpp
  LzHuf.cpp
  rt_compressor.cpp
  rt_lzo.cpp
  crc32.cpp
  xr_shared.cpp
  xrsharedmem.cpp
  xrstring.cpp
  xrSyncronize.cpp
)

add_library(xrCore SHARED ${XRCORE_SOURCES})

set_target_properties(xrCore PROPERTIES
  OUTPUT_NAME "xrCore"
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
)

target_include_directories(xrCore
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
  PRIVATE
    "${CMAKE_SOURCE_DIR}/cmake/compat/msvc"
)

target_compile_definitions(xrCore
  PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    XRCORE_EXPORTS
    _USE_32BIT_TIME_T
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Mixed>:MIXED>
    $<$<CONFIG:DebugNoExceptions>:_DEBUG;FORCE_NO_EXCEPTIONS>
)

if(MSVC)
  target_compile_options(xrCore
    PRIVATE
      /nologo
      /W4
      /GR
      /Gz
      /Zc:forScope-
      $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Gy /Oi- /EHsc>
      $<$<CONFIG:Release>:/O2 /Ob2 /Oi /Ot /GF /GS- /Oy /Zc:wchar_t- /EHs- /EHa->
      $<$<CONFIG:Mixed>:/O2 /Ob2 /Oi /Ot /GF /GS- /EHs- /EHa->
      $<$<CONFIG:DebugNoExceptions>:/Od /Zi /RTC1 /Gy /Oi- /EHs- /EHa->
  )

  target_link_options(xrCore
    PRIVATE
      /nologo
      /MACHINE:I386
      /NODEFAULTLIB:dxerr9.lib
      $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL /MAP>
      $<$<CONFIG:Release>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
      $<$<CONFIG:Mixed>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
      $<$<CONFIG:DebugNoExceptions>:/DEBUG /INCREMENTAL>
  )
endif()

target_precompile_headers(xrCore PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_SOURCE_DIR}/cmake/compat/msvc/xray_msvc_compat.hpp>"
  "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h>"
)

target_link_libraries(xrCore PRIVATE
  winmm
)
