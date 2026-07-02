set(XRLUA_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

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
    "${PROJECT_SOURCE_DIR}/cmake/compat/msvc"
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
