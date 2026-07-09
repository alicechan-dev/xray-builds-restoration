set(XR_LWO_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(XR_LWO_LEGACY_DIR "${PROJECT_SOURCE_DIR}/Editor/Tools/LWO")
set(XR_LWO_PROJECT "${XR_LWO_DIR}/LWO.vcproj")

# Experimental build-probe shell for the historical LightWave object helper
# DLL. The legacy .vcproj remains authoritative for source ownership; missing
# referenced files are resolved from the older Editor/Tools/LWO generation when
# available and reported instead of fabricated when absent from both snapshots.
file(STRINGS "${XR_LWO_PROJECT}" XR_LWO_PROJECT_LINES
  REGEX "RelativePath=\"\\.\\\\[^\"]+\"")

set(XR_LWO_SOURCES)
set(XR_LWO_PROJECT_FILES "${XR_LWO_PROJECT}")
foreach(line IN LISTS XR_LWO_PROJECT_LINES)
  string(REGEX MATCH "RelativePath=\"\\.\\\\([^\"]+)\"" path_match "${line}")
  if(path_match)
    string(REPLACE "\\" "/" relative_path "${CMAKE_MATCH_1}")
    set(source_path "${XR_LWO_DIR}/${relative_path}")
    set(legacy_source_path "${XR_LWO_LEGACY_DIR}/${relative_path}")
    get_filename_component(source_ext "${relative_path}" EXT)
    string(TOLOWER "${source_ext}" source_ext)
    if(source_ext STREQUAL ".cpp" OR source_ext STREQUAL ".c")
      if(EXISTS "${source_path}")
        list(APPEND XR_LWO_SOURCES "${source_path}")
      elseif(EXISTS "${legacy_source_path}")
        list(APPEND XR_LWO_SOURCES "${legacy_source_path}")
        message(STATUS
          "LWO: using legacy Editor/Tools/LWO source '${relative_path}'")
      else()
        message(WARNING "LWO.vcproj references missing source '${relative_path}'")
      endif()
    else()
      if(EXISTS "${source_path}")
        list(APPEND XR_LWO_PROJECT_FILES "${source_path}")
      elseif(EXISTS "${legacy_source_path}")
        list(APPEND XR_LWO_PROJECT_FILES "${legacy_source_path}")
        message(STATUS
          "LWO: using legacy Editor/Tools/LWO file '${relative_path}'")
      else()
        message(WARNING "LWO.vcproj references missing file '${relative_path}'")
      endif()
    endif()
  endif()
endforeach()

if(NOT XR_LWO_SOURCES)
  message(FATAL_ERROR "No sources were imported from LWO.vcproj")
endif()

set_source_files_properties(${XR_LWO_PROJECT_FILES} PROPERTIES
  HEADER_FILE_ONLY TRUE)

add_library(LWO SHARED
  ${XR_LWO_SOURCES}
  ${XR_LWO_PROJECT_FILES}
)

set_target_properties(LWO PROPERTIES
  OUTPUT_NAME "LWO"
  VS_GLOBAL_CharacterSet "MultiByte"
  C_STANDARD 90
  C_STANDARD_REQUIRED NO
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
  set_target_properties(LWO PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_OUTPUT_ROOT}/lib"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(LWO PRIVATE
  "${XR_LWO_DIR}"
  "${XR_LWO_LEGACY_DIR}"
)

target_compile_definitions(LWO PRIVATE
  WIN32
  _WINDOWS
  _USRDLL
  LWO_EXPORTS
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

if(MSVC)
  target_compile_options(LWO PRIVATE
    /nologo
    /W3
    /Gz
    $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob1 /GF>
  )

  target_link_options(LWO PRIVATE
    /nologo
    /MACHINE:I386
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/INCREMENTAL:NO>
  )
endif()
