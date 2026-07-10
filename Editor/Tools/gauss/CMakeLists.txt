set(XR_GAUSS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# The complete VC6 project is the source of truth for this non-GUI filter DLL.
# The later Editors/Tools/gauss snapshot lacks Gauss.cpp and several project
# headers, and also contains unrelated editor scaffolding.
set(XR_GAUSS_SOURCES
  "${XR_GAUSS_DIR}/_compressed_normal.cpp"
  "${XR_GAUSS_DIR}/_math.cpp"
  "${XR_GAUSS_DIR}/_quaternion.cpp"
  "${XR_GAUSS_DIR}/_sphere.cpp"
  "${XR_GAUSS_DIR}/cpuid.cpp"
  "${XR_GAUSS_DIR}/FS.cpp"
  "${XR_GAUSS_DIR}/Gauss.cpp"
  "${XR_GAUSS_DIR}/LzHuf.cpp"
  "${XR_GAUSS_DIR}/StdAfx.cpp"
)

file(GLOB XR_GAUSS_PROJECT_FILES CONFIGURE_DEPENDS
  "${XR_GAUSS_DIR}/*.h"
)
list(APPEND XR_GAUSS_PROJECT_FILES
  "${XR_GAUSS_DIR}/gauss.dsp"
  "${XR_GAUSS_DIR}/ReadMe.txt"
)
set_source_files_properties(${XR_GAUSS_PROJECT_FILES} PROPERTIES
  HEADER_FILE_ONLY TRUE)

add_library(Gauss SHARED
  ${XR_GAUSS_SOURCES}
  ${XR_GAUSS_PROJECT_FILES}
)

set_target_properties(Gauss PROPERTIES
  OUTPUT_NAME "gauss"
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
  set_target_properties(Gauss PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_OUTPUT_ROOT}/lib"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(Gauss PRIVATE
  "${XR_GAUSS_DIR}"
)

set(XR_GAUSS_DX_ROOT "")
foreach(candidate IN ITEMS "${XR_LEGACY_DX_ROOT}" "${XRAY_EFFECTIVE_DXSDK_ROOT}")
  if(candidate)
    set(XR_GAUSS_DX_ROOT "${candidate}")
    break()
  endif()
endforeach()

if(XR_GAUSS_DX_ROOT)
  target_include_directories(Gauss PRIVATE
    "${XR_GAUSS_DX_ROOT}/Include"
    "${XR_GAUSS_DX_ROOT}/include"
  )
  target_link_directories(Gauss PRIVATE
    "${XR_GAUSS_DX_ROOT}/Lib"
    "${XR_GAUSS_DX_ROOT}/lib"
    "${XR_GAUSS_DX_ROOT}/Lib/x86"
    "${XR_GAUSS_DX_ROOT}/lib/x86"
  )
else()
  message(WARNING
    "Gauss requires DirectX 8-era headers and libraries. Set "
    "XR_LEGACY_DX_ROOT or XRAY_DXSDK_ROOT/DXSDK_DIR if they are unavailable.")
endif()

target_compile_definitions(Gauss PRIVATE
  WIN32
  _WINDOWS
  _USRDLL
  GAUSS_EXPORTS
  _CRT_SECURE_NO_WARNINGS
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

target_precompile_headers(Gauss PRIVATE
  "${XR_GAUSS_DIR}/StdAfx.h"
)

if(MSVC)
  target_compile_options(Gauss PRIVATE
    /nologo
    /W3
    $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob1 /GF>
  )

  target_link_options(Gauss PRIVATE
    /nologo
    /MACHINE:I386
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/INCREMENTAL:NO>
  )
endif()

target_link_libraries(Gauss PRIVATE
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
  imagehlp
  winmm
  d3d8
  d3dx8
  dxerr8
  dsound
  dinput
  dxguid
)
