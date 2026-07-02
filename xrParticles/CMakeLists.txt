set(XRPARTICLES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(XRPARTICLES_SOURCES
  "${XRPARTICLES_DIR}/stdafx.cpp"
  "${XRPARTICLES_DIR}/noise.cpp"
  "${XRPARTICLES_DIR}/particle_actions.cpp"
  "${XRPARTICLES_DIR}/particle_actions_collection.cpp"
  "${XRPARTICLES_DIR}/particle_actions_collection_io.cpp"
  "${XRPARTICLES_DIR}/particle_core.cpp"
  "${XRPARTICLES_DIR}/particle_effect.cpp"
  "${XRPARTICLES_DIR}/particle_manager.cpp"
)

add_library(xrParticles SHARED ${XRPARTICLES_SOURCES})

set_target_properties(xrParticles PROPERTIES
  OUTPUT_NAME "xrParticles"
  # xrParticles.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
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
  set_target_properties(xrParticles PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

target_include_directories(xrParticles
  PUBLIC
    "${XRPARTICLES_DIR}"
  PRIVATE
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/cmake/compat/msvc"
)

target_compile_definitions(xrParticles
  PRIVATE
    WIN32
    _WINDOWS
    XR_PARTICLES_EXPORTS
    # xrParticles includes xrCore.h, so it needs the same modern MSVC
    # compatibility switches for legacy xrCore/STL headers even though they
    # were implicit in the original VC2003 environment.
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xrParticles
    PRIVATE
      /nologo
      /W4
      /GR
      /Gz
      /EHs-
      /EHa-
  )

  # Mirrors xrParticles.vcproj optimization/runtime choices where modern MSVC
  # still supports them; obsolete VC2003 settings such as /G7 and /Wp64 are
  # omitted.
  target_compile_options(xrParticles PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Oi->
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy>
  )

  target_link_options(xrParticles PRIVATE
    /nologo
    # stdafx.h and psystem.h have original #pragma comment(lib, "x:\\...").
    # CMake links xrCore directly and builds xrParticles here, so ignore only
    # those hardcoded X: drive default library directives.
    /NODEFAULTLIB:x:\\xrCore.lib
    /NODEFAULTLIB:x:\\xrParticles.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrParticles PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XRPARTICLES_DIR}/stdafx.h>")

target_link_libraries(xrParticles PRIVATE
  xrCore
  odbc32
  odbccp32
)
