set(XRECORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# First modern CMake scaffold for the Borland C++ Builder xrECoreB DLL.
# Keep this target opt-in and partial: the old project includes VCL form units
# and external editor packages that need separate restoration work.
file(GLOB_RECURSE XRECORE_ALL_CPP CONFIGURE_DEPENDS
  "${XRECORE_DIR}/Engine/*.cpp"
  "${XRECORE_DIR}/Editor/*.cpp"
  "${XRECORE_DIR}/stdafx.cpp"
)

set(XRECORE_EXCLUDED_VCL_FORM_SOURCES
  "${XRECORE_DIR}/Editor/ImageEditor.cpp"
  "${XRECORE_DIR}/Editor/LogForm.cpp"
  "${XRECORE_DIR}/Editor/SoundEditor.cpp"
)

set(XRECORE_EXCLUDED_VCL_FORM_RESOURCES
  "${XRECORE_DIR}/Editor/ImageEditor.dfm"
  "${XRECORE_DIR}/Editor/LogForm.dfm"
  "${XRECORE_DIR}/Editor/SoundEditor.dfm"
)

set(XRECORE_SOURCES ${XRECORE_ALL_CPP})
list(REMOVE_ITEM XRECORE_SOURCES ${XRECORE_EXCLUDED_VCL_FORM_SOURCES})

add_library(xrECore SHARED
  ${XRECORE_SOURCES}
  "${XRECORE_DIR}/resource.res"
  "${XRECORE_DIR}/xrECoreB.bpf"
  "${XRECORE_DIR}/xrECoreB.bpr"
)

set_target_properties(xrECore PROPERTIES
  OUTPUT_NAME "xrECore"
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
  set_target_properties(xrECore PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrECore PRIVATE
  "${XRECORE_DIR}"
  "${XRECORE_DIR}/Editor"
  "${XRECORE_DIR}/Engine"
  "${XRECORE_DIR}/Engine/Blenders"
  "${XRECORE_DIR}/Engine/Visuals"
  "${PROJECT_SOURCE_DIR}/xrCore"
  "${PROJECT_SOURCE_DIR}/xr_3da"
  "${PROJECT_SOURCE_DIR}/xr_3da/xrCDB"
  "${PROJECT_SOURCE_DIR}/xrParticles"
  "${PROJECT_SOURCE_DIR}/Editors/xrEProps"
  "${PROJECT_SOURCE_DIR}/Editors/Tools/ETools"
  "${PROJECT_SOURCE_DIR}/cmake/compat/msvc"
  "${PROJECT_SOURCE_DIR}/cmake/compat/vcl"
)

if(XR_EDITOR_ELPACK_INCLUDE_DIR)
  target_include_directories(xrECore PRIVATE "${XR_EDITOR_ELPACK_INCLUDE_DIR}")
  if(IS_DIRECTORY "${XR_ELPACK_ROOT}/Code")
    target_link_directories(xrECore PRIVATE "${XR_ELPACK_ROOT}/Code")
  endif()
endif()

set(XRECORE_DXSDK_ROOT "${XR_LEGACY_DX_ROOT}")
if(NOT XRECORE_DXSDK_ROOT)
  set(XRECORE_DXSDK_ROOT "${XRAY_EFFECTIVE_DXSDK_ROOT}")
endif()

if(XRECORE_DXSDK_ROOT)
  target_include_directories(xrECore PRIVATE
    "${XRECORE_DXSDK_ROOT}/Include"
    "${XRECORE_DXSDK_ROOT}/include"
  )
  target_link_directories(xrECore PRIVATE
    "${XRECORE_DXSDK_ROOT}/Lib"
    "${XRECORE_DXSDK_ROOT}/lib"
    "${XRECORE_DXSDK_ROOT}/Lib/x86"
    "${XRECORE_DXSDK_ROOT}/lib/x86"
  )
else()
  message(WARNING "xrECore includes legacy DirectX 9/D3DX headers. Set XR_LEGACY_DX_ROOT for SDK/editor targets, or use the existing global DirectX SDK variables.")
endif()

target_compile_definitions(xrECore PRIVATE
  WIN32
  _WINDOWS
  _USRDLL
  _EDITOR
  _ECOREB
  _FARQ=
  DIRECTINPUT_VERSION=0x0700
  _HAS_AUTO_PTR_ETC=1
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

if(MSVC)
  target_compile_options(xrECore PRIVATE
    /nologo
    /W3
    /GR
    /Gz
    /EHs-
    /EHa-
    $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /GF /GS->
  )

  target_link_options(xrECore PRIVATE
    /nologo
    /SAFESEH:NO
    /NODEFAULTLIB:libcp.lib
    /NODEFAULTLIB:xrEPropsB.lib
    /NODEFAULTLIB:xrCoreB.lib
    /NODEFAULTLIB:xrSoundB.lib
    /NODEFAULTLIB:xrCDBB.lib
    /NODEFAULTLIB:dinputB.lib
    /NODEFAULTLIB:freeimageB.lib
    /NODEFAULTLIB:dxtB.lib
    /NODEFAULTLIB:xrProgressiveB.lib
    /NODEFAULTLIB:MagicFMDLLB.lib
    /NODEFAULTLIB:EToolsB.lib
    /NODEFAULTLIB:LWOB.lib
    /NODEFAULTLIB:xrHemisphereB.lib
    /NODEFAULTLIB:oggencB.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_link_libraries(xrECore PRIVATE
  xrCore
  xrCDB
  xrParticles
  ETools
  d3d9
  d3dx9
  dinput8
  dsound
  dxguid
)
