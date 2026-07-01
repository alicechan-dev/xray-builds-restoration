set(XRXMLPARSER_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(XRXMLPARSER_SOURCES
  "${XRXMLPARSER_DIR}/stdafx.cpp"
  "${XRXMLPARSER_DIR}/xrXMLParser.cpp"
  "${XRXMLPARSER_DIR}/tinystr.cpp"
  "${XRXMLPARSER_DIR}/tinyxml.cpp"
  "${XRXMLPARSER_DIR}/tinyxmlerror.cpp"
  "${XRXMLPARSER_DIR}/tinyxmlparser.cpp"
)

add_library(xrXMLParser SHARED ${XRXMLPARSER_SOURCES})

set_target_properties(xrXMLParser PROPERTIES
  OUTPUT_NAME "xrXMLParser"
  # xrXMLParser.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:DebugDLL>"
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(xrXMLParser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrXMLParser
  PUBLIC
    "${XRXMLPARSER_DIR}"
  PRIVATE
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/cmake/compat/msvc"
)

target_compile_definitions(xrXMLParser
  PRIVATE
    WIN32
    _WINDOWS
    XRXMLPARSER_EXPORTS
    # xrXMLParser includes xrCore.h, so it needs the same modern MSVC
    # compatibility switches for legacy xrCore/STL headers even though they
    # were implicit in the original VC2003 environment.
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
)

if(MSVC)
  target_compile_options(xrXMLParser
    PRIVATE
      /nologo
      /W4
      /GR
      /EHs-
      /EHa-
      /Gz
      /TP
  )

  # Mirrors xrXMLParser.vcproj optimization/runtime choices where modern MSVC
  # still supports them; obsolete VC2003 settings such as /G7 and /Wp64 are
  # omitted.
  target_compile_options(xrXMLParser PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Gy /Oi->
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy /Zc:wchar_t->
  )

  target_link_options(xrXMLParser PRIVATE
    /nologo
    /MACHINE:I386
    # stdafx.h has the original #pragma comment(lib, "x:\\xrCore.lib").
    # CMake links xrCore directly, so ignore only that hardcoded X: drive
    # default library directive.
    /NODEFAULTLIB:x:\\xrCore.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL /MAP>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrXMLParser PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XRXMLPARSER_DIR}/stdafx.h>")

target_link_libraries(xrXMLParser PRIVATE
  xrCore
  odbc32
  odbccp32
  wininet
  $<$<CONFIG:Debug>:${XRXMLPARSER_DIR}/ChilkatDbgDll.lib>
  $<$<CONFIG:Debug>:${XRXMLPARSER_DIR}/CkBaseDbgDll.lib>
  $<$<NOT:$<CONFIG:Debug>>:${XRXMLPARSER_DIR}/ChilkatRelDll.lib>
  $<$<NOT:$<CONFIG:Debug>>:${XRXMLPARSER_DIR}/CkBaseRelDll.lib>
)
