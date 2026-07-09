set(SHADER_EDITOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SHADER_EDITOR_PROJECT "${SHADER_EDITOR_DIR}/ShaderEditor.bpr")
set(SHADER_EDITOR_LEGACY_MAGICFM_LIB
  "${PROJECT_SOURCE_DIR}/Editor/ShaderEditor/Lib/MagicFM.lib")

# Experimental build-probe shell for the Borland C++ Builder ShaderEditor.exe.
# The .bpr remains authoritative for source ownership; this target does not
# port VCL forms or fake ElPack, AlexMX, MagicFM, or Borland package behavior.
file(STRINGS "${SHADER_EDITOR_PROJECT}" SHADER_EDITOR_PROJECT_LINES
  REGEX "CONTAINERID=\"CCompiler\"")

set(SHADER_EDITOR_SOURCES)
foreach(line IN LISTS SHADER_EDITOR_PROJECT_LINES)
  string(REGEX MATCH "FILENAME=\"([^\"]+)\"" filename_match "${line}")
  if(filename_match)
    string(REPLACE "\\" "/" relative_source "${CMAKE_MATCH_1}")
    if(EXISTS "${SHADER_EDITOR_DIR}/${relative_source}")
      list(APPEND SHADER_EDITOR_SOURCES "${SHADER_EDITOR_DIR}/${relative_source}")
    else()
      message(WARNING
        "ShaderEditor.bpr references missing source '${relative_source}'")
    endif()
  endif()
endforeach()

if(NOT SHADER_EDITOR_SOURCES)
  message(FATAL_ERROR "No C++ sources were imported from ShaderEditor.bpr")
endif()

file(GLOB SHADER_EDITOR_FORM_FILES CONFIGURE_DEPENDS
  "${SHADER_EDITOR_DIR}/*.dfm")

set(SHADER_EDITOR_PROJECT_FILES
  "${SHADER_EDITOR_DIR}/ShaderEditor.bpr"
  "${SHADER_EDITOR_DIR}/ShaderEditor.todo"
  "${SHADER_EDITOR_DIR}/ShaderEditor.res"
  ${SHADER_EDITOR_FORM_FILES}
)

if(EXISTS "${SHADER_EDITOR_DIR}/resource.res")
  list(APPEND SHADER_EDITOR_PROJECT_FILES "${SHADER_EDITOR_DIR}/resource.res")
else()
  message(WARNING "ShaderEditor.bpr references missing resource 'resource.res'")
endif()

set_source_files_properties(${SHADER_EDITOR_PROJECT_FILES} PROPERTIES
  HEADER_FILE_ONLY TRUE)

add_executable(ShaderEditor WIN32
  ${SHADER_EDITOR_SOURCES}
  ${SHADER_EDITOR_PROJECT_FILES}
)

set_target_properties(ShaderEditor PROPERTIES
  OUTPUT_NAME "ShaderEditor"
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(ShaderEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(ShaderEditor PRIVATE
  "${SHADER_EDITOR_DIR}"
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
  target_include_directories(ShaderEditor PRIVATE "${XR_EDITOR_ELPACK_INCLUDE_DIR}")
  if(IS_DIRECTORY "${XR_ELPACK_ROOT}/Code")
    target_link_directories(ShaderEditor PRIVATE "${XR_ELPACK_ROOT}/Code")
  endif()
endif()

if(XR_EDITOR_ALEXMX_INCLUDE_DIR)
  target_include_directories(ShaderEditor PRIVATE "${XR_EDITOR_ALEXMX_INCLUDE_DIR}")
endif()

if(XR_EDITOR_MAGICFM_INCLUDE_DIR)
  target_include_directories(ShaderEditor PRIVATE "${XR_EDITOR_MAGICFM_INCLUDE_DIR}")
endif()

if(XR_EDITOR_BORLAND_INCLUDE_DIR)
  target_include_directories(ShaderEditor PRIVATE "${XR_EDITOR_BORLAND_INCLUDE_DIR}")
endif()

if(XR_EDITOR_BORLAND_VCL_INCLUDE_DIR)
  target_include_directories(ShaderEditor PRIVATE "${XR_EDITOR_BORLAND_VCL_INCLUDE_DIR}")
endif()

set(SHADER_EDITOR_DXSDK_ROOT "${XR_LEGACY_DX_ROOT}")
if(NOT SHADER_EDITOR_DXSDK_ROOT)
  set(SHADER_EDITOR_DXSDK_ROOT "${XRAY_EFFECTIVE_DXSDK_ROOT}")
endif()

if(SHADER_EDITOR_DXSDK_ROOT)
  target_include_directories(ShaderEditor PRIVATE
    "${SHADER_EDITOR_DXSDK_ROOT}/Include"
    "${SHADER_EDITOR_DXSDK_ROOT}/include"
  )
  target_link_directories(ShaderEditor PRIVATE
    "${SHADER_EDITOR_DXSDK_ROOT}/Lib"
    "${SHADER_EDITOR_DXSDK_ROOT}/lib"
    "${SHADER_EDITOR_DXSDK_ROOT}/Lib/x86"
    "${SHADER_EDITOR_DXSDK_ROOT}/lib/x86"
  )
else()
  message(WARNING
    "ShaderEditor requires the historical DirectX 9 SDK. Set "
    "XR_LEGACY_DX_ROOT for this editor-only target.")
endif()

target_compile_definitions(ShaderEditor PRIVATE
  WIN32
  _WINDOWS
  _EDITOR
  _SHADER_EDITOR
  _FARQ=
  DIRECTINPUT_VERSION=0x0700
  _HAS_AUTO_PTR_ETC=1
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

if(MSVC)
  target_compile_options(ShaderEditor PRIVATE
    /nologo
    /W3
    /GR
    /Gz
    /EHs-
    /EHa-
    $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /GF /GS->
  )

  target_link_options(ShaderEditor PRIVATE
    /nologo
    /SAFESEH:NO
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_link_libraries(ShaderEditor PRIVATE
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

if(EXISTS "${SHADER_EDITOR_LEGACY_MAGICFM_LIB}")
  # Historical import library from the older Editor/ generation. This does not
  # replace the missing MagicFM SDK headers or prove ABI compatibility.
  target_link_libraries(ShaderEditor PRIVATE "${SHADER_EDITOR_LEGACY_MAGICFM_LIB}")
else()
  message(WARNING
    "ShaderEditor historical MagicFM import library was not found. "
    "Set XR_MAGICFM_ROOT only after matching SDK headers are available.")
endif()
