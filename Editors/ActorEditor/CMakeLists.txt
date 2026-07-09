set(ACTOR_EDITOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(ACTOR_EDITOR_PROJECT "${ACTOR_EDITOR_DIR}/ActorEditor.bpr")

# Experimental build-probe shell for the Borland C++ Builder ActorEditor.exe.
# The .bpr remains authoritative for source ownership; this target does not
# port VCL forms or fake ElPack, AlexMX, MagicFM, or Borland package behavior.
file(STRINGS "${ACTOR_EDITOR_PROJECT}" ACTOR_EDITOR_PROJECT_LINES
  REGEX "CONTAINERID=\"CCompiler\"")

set(ACTOR_EDITOR_SOURCES)
foreach(line IN LISTS ACTOR_EDITOR_PROJECT_LINES)
  string(REGEX MATCH "FILENAME=\"([^\"]+)\"" filename_match "${line}")
  if(filename_match)
    string(REPLACE "\\" "/" relative_source "${CMAKE_MATCH_1}")
    if(EXISTS "${ACTOR_EDITOR_DIR}/${relative_source}")
      list(APPEND ACTOR_EDITOR_SOURCES "${ACTOR_EDITOR_DIR}/${relative_source}")
    else()
      message(WARNING
        "ActorEditor.bpr references missing source '${relative_source}'")
    endif()
  endif()
endforeach()

if(NOT ACTOR_EDITOR_SOURCES)
  message(FATAL_ERROR "No C++ sources were imported from ActorEditor.bpr")
endif()

file(GLOB ACTOR_EDITOR_FORM_FILES CONFIGURE_DEPENDS
  "${ACTOR_EDITOR_DIR}/*.dfm"
  "${ACTOR_EDITOR_DIR}/*.ddp")

set(ACTOR_EDITOR_PROJECT_FILES
  "${ACTOR_EDITOR_DIR}/ActorEditor.bpr"
  "${ACTOR_EDITOR_DIR}/ActorEditor.todo"
  "${ACTOR_EDITOR_DIR}/ActorEditor.res"
  ${ACTOR_EDITOR_FORM_FILES}
)

if(EXISTS "${ACTOR_EDITOR_DIR}/resource.res")
  list(APPEND ACTOR_EDITOR_PROJECT_FILES "${ACTOR_EDITOR_DIR}/resource.res")
else()
  message(WARNING "ActorEditor.bpr references missing resource 'resource.res'")
endif()

set_source_files_properties(${ACTOR_EDITOR_PROJECT_FILES} PROPERTIES
  HEADER_FILE_ONLY TRUE)

add_executable(ActorEditor WIN32
  ${ACTOR_EDITOR_SOURCES}
  ${ACTOR_EDITOR_PROJECT_FILES}
)

set_target_properties(ActorEditor PROPERTIES
  OUTPUT_NAME "ActorEditor"
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(ActorEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(ActorEditor PRIVATE
  "${ACTOR_EDITOR_DIR}"
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
  target_include_directories(ActorEditor PRIVATE "${XR_EDITOR_ELPACK_INCLUDE_DIR}")
  if(IS_DIRECTORY "${XR_ELPACK_ROOT}/Code")
    target_link_directories(ActorEditor PRIVATE "${XR_ELPACK_ROOT}/Code")
  endif()
endif()

if(XR_EDITOR_ALEXMX_INCLUDE_DIR)
  target_include_directories(ActorEditor PRIVATE "${XR_EDITOR_ALEXMX_INCLUDE_DIR}")
endif()

if(XR_EDITOR_MAGICFM_INCLUDE_DIR)
  target_include_directories(ActorEditor PRIVATE "${XR_EDITOR_MAGICFM_INCLUDE_DIR}")
endif()

if(XR_EDITOR_BORLAND_INCLUDE_DIR)
  target_include_directories(ActorEditor PRIVATE "${XR_EDITOR_BORLAND_INCLUDE_DIR}")
endif()

if(XR_EDITOR_BORLAND_VCL_INCLUDE_DIR)
  target_include_directories(ActorEditor PRIVATE "${XR_EDITOR_BORLAND_VCL_INCLUDE_DIR}")
endif()

set(ACTOR_EDITOR_DXSDK_ROOT "${XR_LEGACY_DX_ROOT}")
if(NOT ACTOR_EDITOR_DXSDK_ROOT)
  set(ACTOR_EDITOR_DXSDK_ROOT "${XRAY_EFFECTIVE_DXSDK_ROOT}")
endif()

if(ACTOR_EDITOR_DXSDK_ROOT)
  target_include_directories(ActorEditor PRIVATE
    "${ACTOR_EDITOR_DXSDK_ROOT}/Include"
    "${ACTOR_EDITOR_DXSDK_ROOT}/include"
  )
  target_link_directories(ActorEditor PRIVATE
    "${ACTOR_EDITOR_DXSDK_ROOT}/Lib"
    "${ACTOR_EDITOR_DXSDK_ROOT}/lib"
    "${ACTOR_EDITOR_DXSDK_ROOT}/Lib/x86"
    "${ACTOR_EDITOR_DXSDK_ROOT}/lib/x86"
  )
else()
  message(WARNING
    "ActorEditor requires the historical DirectX 9 SDK. Set "
    "XR_LEGACY_DX_ROOT for this editor-only target.")
endif()

target_compile_definitions(ActorEditor PRIVATE
  WIN32
  _WINDOWS
  _EDITOR
  _ACTOR_EDITOR
  _FARQ=
  DIRECTINPUT_VERSION=0x0700
  _HAS_AUTO_PTR_ETC=1
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

if(MSVC)
  target_compile_options(ActorEditor PRIVATE
    /nologo
    /W3
    /GR
    /Gz
    /EHs-
    /EHa-
    $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /GF /GS->
  )

  target_link_options(ActorEditor PRIVATE
    /nologo
    /SAFESEH:NO
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_link_libraries(ActorEditor PRIVATE
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

message(STATUS
  "ActorEditor: MagicFMDLLB.lib from the Borland project is not present in "
  "this repository snapshot; existing MagicFM runtime DLLs/libs do not replace "
  "the missing SDK headers or prove ABI compatibility.")
