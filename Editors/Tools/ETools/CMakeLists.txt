set(ETOOLS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(ETOOLS_LEGACY_COLORPICKER_LIB "${PROJECT_SOURCE_DIR}/Editor/Tools/ETools/ColorPicker.lib")

# First SDK-tool CMake pass for the historical ETools DLL.  The original
# ETools.vcproj also built VIPM/progressive-mesh sources that depend on
# x:/xrQSlim.lib.  Leave those out until xrQSlim has its own CMake target.
set(ETOOLS_SOURCES
  "${ETOOLS_DIR}/CreateDX.cpp"
  "${ETOOLS_DIR}/ETools.cpp"
  "${PROJECT_SOURCE_DIR}/cmake/compat/msvc/d3dx9_snprintf_compat.cpp"
)

set(ETOOLS_EXCLUDED_XRQSLIM_SOURCES
  "${ETOOLS_DIR}/object.cpp"
  "${ETOOLS_DIR}/object_sliding.cpp"
  "${ETOOLS_DIR}/optimize_vertex_order.cpp"
  "${ETOOLS_DIR}/PropSlimTools.cpp"
)

add_library(ETools SHARED
  ${ETOOLS_SOURCES}
  "${ETOOLS_DIR}/ArbitraryList.h"
  "${ETOOLS_DIR}/BinaryHeap.h"
  "${ETOOLS_DIR}/D3DX_Wrapper.h"
  "${ETOOLS_DIR}/DLink.h"
  "${ETOOLS_DIR}/ETools.h"
  "${ETOOLS_DIR}/mesh.h"
  "${ETOOLS_DIR}/object.h"
  "${ETOOLS_DIR}/object_sliding.h"
  "${ETOOLS_DIR}/PropSlimTools.h"
  "${ETOOLS_DIR}/quad.h"
  "${ETOOLS_DIR}/stdafx.h"
  "${ETOOLS_DIR}/TomsLib.h"
)

set_target_properties(ETools PROPERTIES
  OUTPUT_NAME "ETools"
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
  set_target_properties(ETools PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(ETools PRIVATE
  "${ETOOLS_DIR}"
  "${PROJECT_SOURCE_DIR}/xrCore"
  "${PROJECT_SOURCE_DIR}/cmake/compat/msvc"
)

set(ETOOLS_DXSDK_ROOT "${XR_LEGACY_DX_ROOT}")
if(NOT ETOOLS_DXSDK_ROOT)
  set(ETOOLS_DXSDK_ROOT "${XRAY_EFFECTIVE_DXSDK_ROOT}")
endif()

if(ETOOLS_DXSDK_ROOT)
  target_include_directories(ETools PRIVATE
    "${ETOOLS_DXSDK_ROOT}/Include"
    "${ETOOLS_DXSDK_ROOT}/include"
  )
  target_link_directories(ETools PRIVATE
    "${ETOOLS_DXSDK_ROOT}/Lib"
    "${ETOOLS_DXSDK_ROOT}/lib"
    "${ETOOLS_DXSDK_ROOT}/Lib/x86"
    "${ETOOLS_DXSDK_ROOT}/lib/x86"
  )
else()
  message(WARNING "ETools includes legacy DirectX 9 D3DX headers. Set XR_LEGACY_DX_ROOT for SDK/editor targets, or use the existing global DirectX SDK variables.")
endif()

target_compile_definitions(ETools PRIVATE
  WIN32
  _WINDOWS
  _USRDLL
  ETOOLS_EXPORTS
  _FARQ=
  _HAS_AUTO_PTR_ETC=1
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

if(MSVC)
  target_compile_options(ETools PRIVATE
    /nologo
    /W3
    /GR
    /Gz
    /EHs-
    /EHa-
    $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /GF /GS->
  )

  target_link_options(ETools PRIVATE
    /nologo
    /SAFESEH:NO
    /NODEFAULTLIB:dxerr9.lib
    /NODEFAULTLIB:libcp.lib
    /NODEFAULTLIB:x:/xrCore.lib
    /NODEFAULTLIB:x:/xrQSlim.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_link_libraries(ETools PRIVATE
  xrCore
  d3dx9
)

if(EXISTS "${ETOOLS_DIR}/ColorPicker.lib")
  target_link_libraries(ETools PRIVATE "${ETOOLS_DIR}/ColorPicker.lib")
elseif(EXISTS "${ETOOLS_LEGACY_COLORPICKER_LIB}")
  # The current Editors/Tools/ETools snapshot references ColorPicker.lib but
  # lacks a local copy.  The older Editor/Tools/ETools helper project contains
  # the matching import library for FSColorPickerDoModal.
  target_link_libraries(ETools PRIVATE "${ETOOLS_LEGACY_COLORPICKER_LIB}")
else()
  message(WARNING "ETools.vcproj referenced ColorPicker.lib, but it was not found beside the project. FSColorPickerExecute may fail to link until that dependency is restored.")
endif()
