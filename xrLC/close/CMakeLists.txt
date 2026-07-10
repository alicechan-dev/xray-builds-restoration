set(XR_CLOSE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Experimental CMake shell for the tiny historical xrLC close helper. This is a
# narrow Win32 mailslot helper and does not pull in the full level compiler.
add_executable(close WIN32
  "${XR_CLOSE_DIR}/stdafx.cpp"
  "${XR_CLOSE_DIR}/close.cpp"
  "${XR_CLOSE_DIR}/close.rc"
  "${XR_CLOSE_DIR}/close.h"
  "${XR_CLOSE_DIR}/Resource.h"
  "${XR_CLOSE_DIR}/close.ico"
  "${XR_CLOSE_DIR}/small.ico"
  "${XR_CLOSE_DIR}/close.vcproj"
  "${XR_CLOSE_DIR}/ReadMe.txt"
)

set_source_files_properties(
  "${XR_CLOSE_DIR}/close.h"
  "${XR_CLOSE_DIR}/Resource.h"
  "${XR_CLOSE_DIR}/close.ico"
  "${XR_CLOSE_DIR}/small.ico"
  "${XR_CLOSE_DIR}/close.vcproj"
  "${XR_CLOSE_DIR}/ReadMe.txt"
  PROPERTIES HEADER_FILE_ONLY TRUE)

set_target_properties(close PROPERTIES
  OUTPUT_NAME "close"
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(close PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(close PRIVATE
  "${XR_CLOSE_DIR}"
  "${PROJECT_SOURCE_DIR}/xrCore"
  "${PROJECT_SOURCE_DIR}/cmake/compat/msvc"
)

target_compile_definitions(close PRIVATE
  WIN32
  _WINDOWS
  _MBCS
  _FARQ=
  _HAS_AUTO_PTR_ETC=1
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
  $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(close PRIVATE
    /nologo
    /W3
    /GR
    /Gz
    /EHs-
    /EHa-
    $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob1 /GF>
  )

  target_link_options(close PRIVATE
    /nologo
    /MACHINE:I386
    "/NODEFAULTLIB:x:\\xrCore.lib"
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_link_libraries(close PRIVATE
  xrCore
)
