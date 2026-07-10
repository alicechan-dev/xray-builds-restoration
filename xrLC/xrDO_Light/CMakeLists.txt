set(XR_DO_LIGHT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(XR_DO_LIGHT_FREEIMAGE_LIB
  "${PROJECT_SOURCE_DIR}/Editor/ShaderEditor/Lib/FreeImage.lib"
  CACHE FILEPATH
  "Optional MSVC-compatible FreeImage import library for xrDO_Light")

# Experimental CMake shell for the historical detail-object light compiler
# helper. The source list mirrors xrDO_Light.vcproj and intentionally avoids
# pulling in the broader xrLC compiler tree.
set(XR_DO_LIGHT_SOURCES
  "${XR_DO_LIGHT_DIR}/StdAfx.cpp"
  "${XR_DO_LIGHT_DIR}/tga.cpp"
  "${XR_DO_LIGHT_DIR}/xrThread.cpp"
  "${XR_DO_LIGHT_DIR}/cl_log.cpp"
  "${XR_DO_LIGHT_DIR}/compiler.cpp"
  "${XR_DO_LIGHT_DIR}/xrDO_Light.cpp"
  "${XR_DO_LIGHT_DIR}/xrLoadSurface.cpp"
  "${XR_DO_LIGHT_DIR}/resource.rc"
)

set(XR_DO_LIGHT_PROJECT_FILES
  "${XR_DO_LIGHT_DIR}/cl_intersect.h"
  "${XR_DO_LIGHT_DIR}/cl_log.h"
  "${XR_DO_LIGHT_DIR}/communicate.h"
  "${XR_DO_LIGHT_DIR}/defines.h"
  "${XR_DO_LIGHT_DIR}/DetailFormat.h"
  "${XR_DO_LIGHT_DIR}/ETextureParams.h"
  "${XR_DO_LIGHT_DIR}/FreeImage.h"
  "${XR_DO_LIGHT_DIR}/light.h"
  "${XR_DO_LIGHT_DIR}/resource.h"
  "${XR_DO_LIGHT_DIR}/R_light.h"
  "${XR_DO_LIGHT_DIR}/Shader_xrLC.h"
  "${XR_DO_LIGHT_DIR}/StdAfx.h"
  "${XR_DO_LIGHT_DIR}/std_classes.h"
  "${XR_DO_LIGHT_DIR}/tga.h"
  "${XR_DO_LIGHT_DIR}/xrCDB.h"
  "${XR_DO_LIGHT_DIR}/xrDO_Light.dsp"
  "${XR_DO_LIGHT_DIR}/xrDO_Light.vcproj"
  "${XR_DO_LIGHT_DIR}/xrHemisphere.h"
  "${XR_DO_LIGHT_DIR}/xrLevel.h"
  "${XR_DO_LIGHT_DIR}/xrShaderTypes.h"
  "${XR_DO_LIGHT_DIR}/xrSyncronize.h"
  "${XR_DO_LIGHT_DIR}/xrThread.h"
)

if(NOT EXISTS "${XR_DO_LIGHT_FREEIMAGE_LIB}")
  message(FATAL_ERROR
    "xrDO_Light requires the historical FreeImage import library, but it was "
    "not found at '${XR_DO_LIGHT_FREEIMAGE_LIB}'")
endif()

message(STATUS "xrDO_Light FreeImage import library: ${XR_DO_LIGHT_FREEIMAGE_LIB}")

set_source_files_properties(${XR_DO_LIGHT_PROJECT_FILES} PROPERTIES
  HEADER_FILE_ONLY TRUE)

add_executable(xrDO_Light WIN32
  ${XR_DO_LIGHT_SOURCES}
  ${XR_DO_LIGHT_PROJECT_FILES}
)

set_target_properties(xrDO_Light PROPERTIES
  OUTPUT_NAME "xrDO_Light"
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(xrDO_Light PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrDO_Light PRIVATE
  "${XR_DO_LIGHT_DIR}"
  "${PROJECT_SOURCE_DIR}/xrCore"
  "${PROJECT_SOURCE_DIR}/xr_3da/xrCDB"
  "${PROJECT_SOURCE_DIR}/cmake/compat/msvc"
)

set(XR_DO_LIGHT_DX_ROOT "")
foreach(candidate IN ITEMS "${XR_LEGACY_DX_ROOT}" "${XRAY_EFFECTIVE_DXSDK_ROOT}")
  if(candidate)
    set(XR_DO_LIGHT_DX_ROOT "${candidate}")
    break()
  endif()
endforeach()

if(XR_DO_LIGHT_DX_ROOT)
  target_include_directories(xrDO_Light PRIVATE
    "${XR_DO_LIGHT_DX_ROOT}/Include"
    "${XR_DO_LIGHT_DX_ROOT}/include"
  )
  target_link_directories(xrDO_Light PRIVATE
    "${XR_DO_LIGHT_DX_ROOT}/Lib"
    "${XR_DO_LIGHT_DX_ROOT}/lib"
    "${XR_DO_LIGHT_DX_ROOT}/Lib/x86"
    "${XR_DO_LIGHT_DX_ROOT}/lib/x86"
  )
else()
  message(WARNING
    "xrDO_Light includes legacy DirectX header d3dx9.h. Set XR_LEGACY_DX_ROOT "
    "or XRAY_DXSDK_ROOT/DXSDK_DIR if the active SDK does not provide it.")
endif()

target_compile_definitions(xrDO_Light PRIVATE
  WIN32
  _WINDOWS
  WINVER=0x0501
  _WIN32_WINNT=0x0501
  _CRT_SECURE_NO_WARNINGS
  _FARQ=
  _HAS_AUTO_PTR_ETC=1
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
  $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xrDO_Light PRIVATE
    /nologo
    /W3
    /GR
    /Gz
    /EHs-
    /EHa-
    $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob1 /GF>
  )

  target_link_options(xrDO_Light PRIVATE
    /nologo
    /MACHINE:I386
    "/NODEFAULTLIB:X:\\xrHemisphere.lib"
    "/NODEFAULTLIB:X:\\xrCDB.lib"
    "/NODEFAULTLIB:X:\\xrCore.lib"
    "/NODEFAULTLIB:X:\\FreeImage.lib"
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrDO_Light PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:${XR_DO_LIGHT_DIR}/stdafx.h>")

target_link_libraries(xrDO_Light PRIVATE
  xrCore
  xrCDB
  xrHemisphere
  "${XR_DO_LIGHT_FREEIMAGE_LIB}"
  comctl32
  winmm
  d3dx9
  kernel32
  user32
  gdi32
  winspool
  comdlg32
  advapi32
  shell32
  ole32
  oleaut32
  uuid
  odbc32
  odbccp32
)
