set(XR_DXT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(XR_DXT_PROJECT "${XR_DXT_DIR}/DXT.dsp")
set(XR_DXT_NVIDIA_LIB "${XR_DXT_DIR}/nvDXTlib.lib")

# Experimental build-probe shell for the historical DXT texture helper DLL.
# The VC6 project remains the source-of-truth: nvdxt.cpp is intentionally not
# compiled because the project marked it Exclude_From_Build and the DLL links
# the bundled historical nvDXTlib.lib instead.
set(XR_DXT_SOURCES
  "${XR_DXT_DIR}/StdAfx.cpp"
  "${XR_DXT_DIR}/DXT.cpp"
  "${XR_DXT_DIR}/ETextureParams.cpp"
  "${XR_DXT_DIR}/FS.cpp"
  "${XR_DXT_DIR}/LzHuf.cpp"
  "${XR_DXT_DIR}/msvc_legacy_stdio.cpp"
)

set(XR_DXT_PROJECT_FILES
  "${XR_DXT_PROJECT}"
  "${XR_DXT_DIR}/dds.h"
  "${XR_DXT_DIR}/dxt.h"
  "${XR_DXT_DIR}/dxtlib.h"
  "${XR_DXT_DIR}/ETextureParams.h"
  "${XR_DXT_DIR}/FS.h"
  "${XR_DXT_DIR}/lzhuf.h"
  "${XR_DXT_DIR}/nvdxt.cpp"
  "${XR_DXT_DIR}/nvdxt_options.h"
  "${XR_DXT_NVIDIA_LIB}"
  "${XR_DXT_DIR}/ReadMe.txt"
  "${XR_DXT_DIR}/StdAfx.h"
  "${XR_DXT_DIR}/xr_tokens.h"
)

if(NOT EXISTS "${XR_DXT_NVIDIA_LIB}")
  message(FATAL_ERROR "DXT requires bundled historical nvDXTlib.lib, but it was not found at '${XR_DXT_NVIDIA_LIB}'")
endif()

set_source_files_properties(${XR_DXT_PROJECT_FILES} PROPERTIES
  HEADER_FILE_ONLY TRUE)

add_library(DXT SHARED
  ${XR_DXT_SOURCES}
  ${XR_DXT_PROJECT_FILES}
)

set_target_properties(DXT PROPERTIES
  OUTPUT_NAME "DXT"
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
  set_target_properties(DXT PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_OUTPUT_ROOT}/lib"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(DXT PRIVATE
  "${XR_DXT_DIR}"
)

set(XR_DXT_DX_ROOT "")
foreach(candidate IN ITEMS "${XR_LEGACY_DX_ROOT}" "${XRAY_EFFECTIVE_DXSDK_ROOT}")
  if(candidate)
    set(XR_DXT_DX_ROOT "${candidate}")
    break()
  endif()
endforeach()

if(XR_DXT_DX_ROOT)
  target_include_directories(DXT PRIVATE
    "${XR_DXT_DX_ROOT}/Include"
    "${XR_DXT_DX_ROOT}/include"
  )
  target_link_directories(DXT PRIVATE
    "${XR_DXT_DX_ROOT}/Lib"
    "${XR_DXT_DX_ROOT}/lib"
    "${XR_DXT_DX_ROOT}/Lib/x86"
    "${XR_DXT_DX_ROOT}/lib/x86"
  )
else()
  message(WARNING
    "DXT includes legacy DirectX header d3dx.h. Set XR_LEGACY_DX_ROOT "
    "or XRAY_DXSDK_ROOT/DXSDK_DIR if the active SDK does not provide it.")
endif()

target_compile_definitions(DXT PRIVATE
  WIN32
  _WINDOWS
  _USRDLL
  DXT_EXPORTS
  _CRT_SECURE_NO_WARNINGS
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

if(MSVC)
  target_compile_options(DXT PRIVATE
    /nologo
    /W3
    $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob1 /GF>
  )

  target_link_options(DXT PRIVATE
    /nologo
    /MACHINE:I386
    # The bundled VC6-era nvDXTlib.lib has no SAFESEH metadata and carries an
    # obsolete VC6 CRT default-library requests; keep this compatibility local
    # to the historical DXT helper.
    /SAFESEH:NO
    /NODEFAULTLIB:libcp.lib
    /NODEFAULTLIB:LIBC.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/INCREMENTAL:NO>
  )
endif()

target_link_libraries(DXT PRIVATE
  "${XR_DXT_NVIDIA_LIB}"
  legacy_stdio_definitions
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
