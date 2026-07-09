set(LEVEL_OPTIONS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Experimental build-probe shell for the Borland C++ Builder xrLC_Options DLL.
# This target keeps the historical source/form ownership visible without
# pretending that the VCL, ElPack, AlexMX, or MagicFM GUI dependencies exist.
set(LEVEL_OPTIONS_SOURCES
  "${LEVEL_OPTIONS_DIR}/stdafx.cpp"
  "${LEVEL_OPTIONS_DIR}/Editor/SceneProperties.cpp"
)

set(LEVEL_OPTIONS_PROJECT_FILES
  "${LEVEL_OPTIONS_DIR}/xrLC_Options.bpf"
  "${LEVEL_OPTIONS_DIR}/xrLC_Options.bpr"
  "${LEVEL_OPTIONS_DIR}/Editor/SceneProperties.dfm"
)
set_source_files_properties(${LEVEL_OPTIONS_PROJECT_FILES} PROPERTIES
  HEADER_FILE_ONLY TRUE)

if(NOT EXISTS "${LEVEL_OPTIONS_DIR}/xrLC_OptionsEntry.cpp")
  message(WARNING
    "xrLC_Options.bpr references missing source 'xrLC_OptionsEntry.cpp'; "
    "LevelOptions remains an experimental dependency shell.")
endif()

add_library(LevelOptions SHARED
  ${LEVEL_OPTIONS_SOURCES}
  ${LEVEL_OPTIONS_PROJECT_FILES}
)

set_target_properties(LevelOptions PROPERTIES
  OUTPUT_NAME "xrLC_Options"
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
  set_target_properties(LevelOptions PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(LevelOptions PRIVATE
  "${LEVEL_OPTIONS_DIR}"
  "${LEVEL_OPTIONS_DIR}/Editor"
  "${PROJECT_SOURCE_DIR}/xrCore"
  "${PROJECT_SOURCE_DIR}/xr_3da"
  "${PROJECT_SOURCE_DIR}/xr_3da/xrCDB"
  "${PROJECT_SOURCE_DIR}/xrParticles"
  "${PROJECT_SOURCE_DIR}/Editors/ECore"
  "${PROJECT_SOURCE_DIR}/Editors/ECore/Editor"
  "${PROJECT_SOURCE_DIR}/Editors/ECore/Engine"
  "${PROJECT_SOURCE_DIR}/Editors/ECore/Engine/Blenders"
  "${PROJECT_SOURCE_DIR}/Editors/ECore/Engine/Visuals"
  "${PROJECT_SOURCE_DIR}/Editors/xrEProps"
  "${PROJECT_SOURCE_DIR}/Editors/Tools/ETools"
  "${PROJECT_SOURCE_DIR}/cmake/compat/msvc"
  "${PROJECT_SOURCE_DIR}/cmake/compat/vcl"
)

if(XR_EDITOR_ELPACK_INCLUDE_DIR)
  target_include_directories(LevelOptions PRIVATE "${XR_EDITOR_ELPACK_INCLUDE_DIR}")
  if(IS_DIRECTORY "${XR_ELPACK_ROOT}/Code")
    target_link_directories(LevelOptions PRIVATE "${XR_ELPACK_ROOT}/Code")
  endif()
endif()

if(XR_EDITOR_ALEXMX_INCLUDE_DIR)
  target_include_directories(LevelOptions PRIVATE "${XR_EDITOR_ALEXMX_INCLUDE_DIR}")
endif()

if(XR_EDITOR_MAGICFM_INCLUDE_DIR)
  target_include_directories(LevelOptions PRIVATE "${XR_EDITOR_MAGICFM_INCLUDE_DIR}")
endif()

if(XR_EDITOR_BORLAND_INCLUDE_DIR)
  target_include_directories(LevelOptions PRIVATE "${XR_EDITOR_BORLAND_INCLUDE_DIR}")
endif()

if(XR_EDITOR_BORLAND_VCL_INCLUDE_DIR)
  target_include_directories(LevelOptions PRIVATE "${XR_EDITOR_BORLAND_VCL_INCLUDE_DIR}")
endif()

set(LEVEL_OPTIONS_DXSDK_ROOT "${XR_LEGACY_DX_ROOT}")
if(NOT LEVEL_OPTIONS_DXSDK_ROOT)
  set(LEVEL_OPTIONS_DXSDK_ROOT "${XRAY_EFFECTIVE_DXSDK_ROOT}")
endif()

if(LEVEL_OPTIONS_DXSDK_ROOT)
  target_include_directories(LevelOptions PRIVATE
    "${LEVEL_OPTIONS_DXSDK_ROOT}/Include"
    "${LEVEL_OPTIONS_DXSDK_ROOT}/include"
  )
  target_link_directories(LevelOptions PRIVATE
    "${LEVEL_OPTIONS_DXSDK_ROOT}/Lib"
    "${LEVEL_OPTIONS_DXSDK_ROOT}/lib"
    "${LEVEL_OPTIONS_DXSDK_ROOT}/Lib/x86"
    "${LEVEL_OPTIONS_DXSDK_ROOT}/lib/x86"
  )
else()
  message(WARNING
    "LevelOptions requires the historical DirectX 9 SDK. Set "
    "XR_LEGACY_DX_ROOT for this editor-only target.")
endif()

target_compile_definitions(LevelOptions PRIVATE
  WIN32
  _WINDOWS
  _USRDLL
  _EDITOR
  _LEVEL_OPTIONS
  _FARQ=
  DIRECTINPUT_VERSION=0x0700
  _HAS_AUTO_PTR_ETC=1
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

if(MSVC)
  target_compile_options(LevelOptions PRIVATE
    /nologo
    /W3
    /GR
    /Gz
    /EHs-
    /EHa-
    $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /GF /GS->
  )

  target_link_options(LevelOptions PRIVATE
    /nologo
    /SAFESEH:NO
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_link_libraries(LevelOptions PRIVATE
  xrCore
  xrCDB
  xrParticles
  xrSound
  ETools
  xrECore
  xrEProps
  d3d9
  d3dx9
  dinput8
  dsound
  dxguid
)
