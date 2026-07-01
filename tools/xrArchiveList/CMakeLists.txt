add_executable(xrArchiveList
  xrArchiveList.cpp
  "${PROJECT_SOURCE_DIR}/xrCore/LzHuf.cpp"
)

set_target_properties(xrArchiveList PROPERTIES
  OUTPUT_NAME "xrArchiveList"
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(xrArchiveList PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrArchiveList PRIVATE
  "${PROJECT_SOURCE_DIR}/xrCore"
  "${PROJECT_SOURCE_DIR}/cmake/compat/msvc"
)

target_compile_definitions(xrArchiveList PRIVATE
  WIN32
  _CONSOLE
  _FARQ=
  _HAS_AUTO_PTR_ETC=1
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
  $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xrArchiveList PRIVATE
    /nologo
    /W3
    /GR
    /Gz
    /EHsc
  )

  target_link_options(xrArchiveList PRIVATE
    /nologo
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_link_libraries(xrArchiveList PRIVATE
  xrCore
)
