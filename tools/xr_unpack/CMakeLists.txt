add_executable(xr_unpack
  src/main.cpp
  src/archive_reader.cpp
  src/archive_reader.h
  src/path_safety.cpp
  src/path_safety.h
  "${PROJECT_SOURCE_DIR}/xrCore/LzHuf.cpp"
)

set_target_properties(xr_unpack PROPERTIES
  OUTPUT_NAME "xr_unpack"
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(xr_unpack PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xr_unpack PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${PROJECT_SOURCE_DIR}/xrCore"
  "${PROJECT_SOURCE_DIR}/cmake/compat/msvc"
)

target_compile_definitions(xr_unpack PRIVATE
  WIN32
  _CONSOLE
  _FARQ=
  _HAS_AUTO_PTR_ETC=1
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
  $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xr_unpack PRIVATE
    /nologo
    /W3
    /GR
    /Gz
    /EHsc
  )

  target_link_options(xr_unpack PRIVATE
    /nologo
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_link_libraries(xr_unpack PRIVATE
  xrCore
)
