set(XR_HEMISPHERE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Experimental CMake shell for the small historical hemisphere sampling helper
# DLL used by the level compiler toolchain. The source is intentionally kept to
# the original project entries; no broader xrLC restoration is pulled in here.
add_library(xrHemisphere SHARED
  "${XR_HEMISPHERE_DIR}/StdAfx.cpp"
  "${XR_HEMISPHERE_DIR}/xrHemisphere.cpp"
  "${XR_HEMISPHERE_DIR}/xrHemisphere.h"
  "${XR_HEMISPHERE_DIR}/xrHemisphere.vcproj"
  "${XR_HEMISPHERE_DIR}/xrHemisphere.dsp"
)

set_source_files_properties(
  "${XR_HEMISPHERE_DIR}/xrHemisphere.h"
  "${XR_HEMISPHERE_DIR}/xrHemisphere.vcproj"
  "${XR_HEMISPHERE_DIR}/xrHemisphere.dsp"
  PROPERTIES HEADER_FILE_ONLY TRUE)

set_target_properties(xrHemisphere PROPERTIES
  OUTPUT_NAME "xrHemisphere"
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${XRAY_OUTPUT_ROOT}/lib"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(xrHemisphere PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_OUTPUT_ROOT}/lib"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrHemisphere PRIVATE
  "${XR_HEMISPHERE_DIR}"
  "${PROJECT_SOURCE_DIR}/xrCore"
  "${PROJECT_SOURCE_DIR}/cmake/compat/msvc"
)

target_compile_definitions(xrHemisphere PRIVATE
  WIN32
  _WINDOWS
  _USRDLL
  XRHEMISPHERE_EXPORTS
  _FARQ=
  _HAS_AUTO_PTR_ETC=1
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
  $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xrHemisphere PRIVATE
    /nologo
    /W3
    /GR
    /Gz
    /EHs-
    /EHa-
    $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /GF /GS->
  )

  target_link_options(xrHemisphere PRIVATE
    /nologo
    /MACHINE:I386
    "/NODEFAULTLIB:x:\\xrCore.lib"
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_link_libraries(xrHemisphere PRIVATE
  xrCore
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
