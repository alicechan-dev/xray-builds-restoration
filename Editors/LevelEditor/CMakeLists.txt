set(LEVEL_EDITOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(LEVEL_EDITOR_PROJECT "${LEVEL_EDITOR_DIR}/LevelEditor.bpr")

# Keep the Borland project authoritative: import only units marked as CCompiler
# inputs instead of treating every historical source in the directory as live.
file(STRINGS "${LEVEL_EDITOR_PROJECT}" LEVEL_EDITOR_PROJECT_LINES
  REGEX "CONTAINERID=\"CCompiler\"")

set(LEVEL_EDITOR_SOURCES)
foreach(line IN LISTS LEVEL_EDITOR_PROJECT_LINES)
  string(REGEX MATCH "FILENAME=\"([^\"]+)\"" filename_match "${line}")
  if(filename_match)
    string(REPLACE "\\" "/" relative_source "${CMAKE_MATCH_1}")
    if(EXISTS "${LEVEL_EDITOR_DIR}/${relative_source}")
      list(APPEND LEVEL_EDITOR_SOURCES "${LEVEL_EDITOR_DIR}/${relative_source}")
    else()
      message(WARNING
        "LevelEditor.bpr references missing source '${relative_source}'")
    endif()
  endif()
endforeach()

if(NOT LEVEL_EDITOR_SOURCES)
  message(FATAL_ERROR "No C++ sources were imported from LevelEditor.bpr")
endif()

file(GLOB_RECURSE LEVEL_EDITOR_FORM_FILES CONFIGURE_DEPENDS
  "${LEVEL_EDITOR_DIR}/*.dfm")
set(LEVEL_EDITOR_PROJECT_FILES
  "${LEVEL_EDITOR_DIR}/LevelEditor.bpr"
  "${LEVEL_EDITOR_DIR}/LevelEditor.res"
  ${LEVEL_EDITOR_FORM_FILES}
)
if(EXISTS "${LEVEL_EDITOR_DIR}/resource.res")
  list(APPEND LEVEL_EDITOR_PROJECT_FILES "${LEVEL_EDITOR_DIR}/resource.res")
else()
  message(WARNING "LevelEditor.bpr references missing resource 'resource.res'")
endif()
set_source_files_properties(${LEVEL_EDITOR_PROJECT_FILES} PROPERTIES
  HEADER_FILE_ONLY TRUE)

add_executable(LevelEditor WIN32
  ${LEVEL_EDITOR_SOURCES}
  ${LEVEL_EDITOR_PROJECT_FILES}
)

set_target_properties(LevelEditor PROPERTIES
  OUTPUT_NAME "LevelEditor"
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(LevelEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(LevelEditor PRIVATE
  "${LEVEL_EDITOR_DIR}"
  "${LEVEL_EDITOR_DIR}/Edit"
  "${LEVEL_EDITOR_DIR}/Engine"
  "${LEVEL_EDITOR_DIR}/Engine/Game"
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

if(XRAY_BOOST_ROOT)
  target_include_directories(LevelEditor PRIVATE "${XRAY_BOOST_ROOT}")
endif()

if(XR_EDITOR_ELPACK_INCLUDE_DIR)
  target_include_directories(LevelEditor PRIVATE
    "${XR_EDITOR_ELPACK_INCLUDE_DIR}")
  if(IS_DIRECTORY "${XR_ELPACK_ROOT}/Code")
    target_link_directories(LevelEditor PRIVATE "${XR_ELPACK_ROOT}/Code")
  endif()
endif()

set(LEVEL_EDITOR_DXSDK_ROOT "${XR_LEGACY_DX_ROOT}")
if(NOT LEVEL_EDITOR_DXSDK_ROOT)
  set(LEVEL_EDITOR_DXSDK_ROOT "${XRAY_EFFECTIVE_DXSDK_ROOT}")
endif()

if(LEVEL_EDITOR_DXSDK_ROOT)
  target_include_directories(LevelEditor PRIVATE
    "${LEVEL_EDITOR_DXSDK_ROOT}/Include"
    "${LEVEL_EDITOR_DXSDK_ROOT}/include"
  )
  target_link_directories(LevelEditor PRIVATE
    "${LEVEL_EDITOR_DXSDK_ROOT}/Lib"
    "${LEVEL_EDITOR_DXSDK_ROOT}/lib"
    "${LEVEL_EDITOR_DXSDK_ROOT}/Lib/x86"
    "${LEVEL_EDITOR_DXSDK_ROOT}/lib/x86"
  )
else()
  message(WARNING
    "LevelEditor requires the historical DirectX 9 SDK. Set "
    "XR_LEGACY_DX_ROOT for this editor-only target.")
endif()

target_compile_definitions(LevelEditor PRIVATE
  WIN32
  _WINDOWS
  _EDITOR
  _LEVEL_EDITOR
  _FARQ=
  DIRECTINPUT_VERSION=0x0700
  _HAS_AUTO_PTR_ETC=1
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

if(MSVC)
  target_compile_options(LevelEditor PRIVATE
    /nologo
    /W3
    /GR
    /Gz
    /EHs-
    /EHa-
    $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /GF /GS->
  )

  target_link_options(LevelEditor PRIVATE
    /nologo
    /SAFESEH:NO
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_link_libraries(LevelEditor PRIVATE
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
