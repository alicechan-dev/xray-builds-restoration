set(XRNETSERVER_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(XRNETSERVER_SOURCES
  "${XRNETSERVER_DIR}/NET_Client.cpp"
  "${XRNETSERVER_DIR}/NET_Compressor.cpp"
  "${XRNETSERVER_DIR}/NET_Server.cpp"
  "${XRNETSERVER_DIR}/stdafx.cpp"
)

add_library(xrNetServer SHARED ${XRNETSERVER_SOURCES})

set_target_properties(xrNetServer PROPERTIES
  OUTPUT_NAME "xrNetServer"
  # xrNetServer.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
  VS_GLOBAL_CharacterSet "MultiByte"
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${XRAY_LIBRARY_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
  INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(xrNetServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrNetServer
  PUBLIC
    "${XRNETSERVER_DIR}"
  PRIVATE
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/cmake/compat/msvc"
)

if(XRAY_EFFECTIVE_DXSDK_ROOT)
  target_include_directories(xrNetServer PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Include" "${XRAY_EFFECTIVE_DXSDK_ROOT}/include")
  target_link_directories(xrNetServer PRIVATE "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib" "${XRAY_EFFECTIVE_DXSDK_ROOT}/Lib/x86" "${XRAY_EFFECTIVE_DXSDK_ROOT}/lib/x86")
endif()

target_compile_definitions(xrNetServer
  PRIVATE
    WIN32
    _WINDOWS
    XR_NETSERVER_EXPORTS
    # xrNetServer includes xrCore.h, so it needs the same modern MSVC
    # compatibility switches for legacy xrCore/STL headers even though they
    # were implicit in the original VC2003 environment.
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xrNetServer
    PRIVATE
      /nologo
      /W4
      /GR
      /Gz
      /EHs-
      /EHa-
  )

  # Mirrors xrNetServer.vcproj optimization/runtime choices where modern MSVC
  # still supports them; obsolete VC2003 settings such as /G7 and /Wp64 are
  # omitted.
  target_compile_options(xrNetServer PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Oi->
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy>
  )

  target_link_options(xrNetServer PRIVATE
    /nologo
    # stdafx.cpp and NET_Shared.h have original #pragma comment(lib, "x:\\...").
    # CMake links xrCore directly and builds xrNetServer here, so ignore only
    # those hardcoded X: drive default library directives.
    /NODEFAULTLIB:x:\\xrCore.lib
    /NODEFAULTLIB:x:\\xrNetServer.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrNetServer PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XRNETSERVER_DIR}/stdafx.h>")

target_link_libraries(xrNetServer PRIVATE
  xrCore
  odbc32
  odbccp32
  dxguid
)
