set(XREPROPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Build-probe scaffold for the Borland C++ Builder xrEPropsB DLL. The source
# set intentionally retains its VCL forms and ElPack-based property UI.
set(XREPROPS_SOURCES
  "${XREPROPS_DIR}/stdafx.cpp"
  "${XREPROPS_DIR}/PropertiesListTypes.cpp"
  "${XREPROPS_DIR}/ItemList.cpp"
  "${XREPROPS_DIR}/ItemListHelper.cpp"
  "${XREPROPS_DIR}/ItemListTypes.cpp"
  "${XREPROPS_DIR}/PropertiesList.cpp"
  "${XREPROPS_DIR}/PropertiesListHelper.cpp"
  "${XREPROPS_DIR}/FolderLib.cpp"
  "${XREPROPS_DIR}/ColorPicker.cpp"
  "${XREPROPS_DIR}/NumericVector.cpp"
  "${XREPROPS_DIR}/ShaderFunction.cpp"
  "${XREPROPS_DIR}/TextForm.cpp"
  "${XREPROPS_DIR}/xrEProps.cpp"
  "${XREPROPS_DIR}/ChoseForm.cpp"
  "${XREPROPS_DIR}/ItemDialog.cpp"
)

set(XREPROPS_PROJECT_FILES
  "${XREPROPS_DIR}/xrEPropsB.bpf"
  "${XREPROPS_DIR}/xrEPropsB.bpr"
  "${XREPROPS_DIR}/ChoseForm.dfm"
  "${XREPROPS_DIR}/ItemDialog.dfm"
  "${XREPROPS_DIR}/ItemList.dfm"
  "${XREPROPS_DIR}/NumericVector.dfm"
  "${XREPROPS_DIR}/PropertiesList.dfm"
  "${XREPROPS_DIR}/ShaderFunction.dfm"
  "${XREPROPS_DIR}/TextForm.dfm"
)
set_source_files_properties(${XREPROPS_PROJECT_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

add_library(xrEProps SHARED
  ${XREPROPS_SOURCES}
  ${XREPROPS_PROJECT_FILES}
  "${XREPROPS_DIR}/xrEPropsB.res"
)

set_target_properties(xrEProps PROPERTIES
  OUTPUT_NAME "xrEProps"
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
  set_target_properties(xrEProps PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrEProps PRIVATE
  "${XREPROPS_DIR}"
  "${PROJECT_SOURCE_DIR}/xrCore"
  "${PROJECT_SOURCE_DIR}/Editors/Tools/ETools"
  "${PROJECT_SOURCE_DIR}/cmake/compat/msvc"
  "${PROJECT_SOURCE_DIR}/cmake/compat/vcl"
)

if(XR_EDITOR_ELPACK_INCLUDE_DIR)
  target_include_directories(xrEProps PRIVATE "${XR_EDITOR_ELPACK_INCLUDE_DIR}")
  if(IS_DIRECTORY "${XR_ELPACK_ROOT}/Code")
    target_link_directories(xrEProps PRIVATE "${XR_ELPACK_ROOT}/Code")
  endif()
endif()

if(XR_EDITOR_ALEXMX_INCLUDE_DIR)
  target_include_directories(xrEProps PRIVATE "${XR_EDITOR_ALEXMX_INCLUDE_DIR}")
endif()

if(XR_EDITOR_MAGICFM_INCLUDE_DIR)
  target_include_directories(xrEProps PRIVATE "${XR_EDITOR_MAGICFM_INCLUDE_DIR}")
endif()

if(XR_EDITOR_BORLAND_INCLUDE_DIR)
  target_include_directories(xrEProps PRIVATE "${XR_EDITOR_BORLAND_INCLUDE_DIR}")
endif()

if(XR_EDITOR_BORLAND_VCL_INCLUDE_DIR)
  target_include_directories(xrEProps PRIVATE "${XR_EDITOR_BORLAND_VCL_INCLUDE_DIR}")
endif()

target_compile_definitions(xrEProps PRIVATE
  WIN32
  _WINDOWS
  _USRDLL
  _EDITOR
  XR_EPROPS_EXPORTS
  _FARQ=
  _HAS_AUTO_PTR_ETC=1
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

if(MSVC)
  target_compile_options(xrEProps PRIVATE
    /nologo
    /W3
    /GR
    /Gz
    /EHs-
    /EHa-
    $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /GF /GS->
  )

  target_link_options(xrEProps PRIVATE
    /nologo
    /SAFESEH:NO
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_link_libraries(xrEProps PRIVATE
  xrCore
  ETools
)
