set(XRCDB_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(XRCDB_SOURCES
  "${XRCDB_DIR}/Frustum.cpp"
  "${XRCDB_DIR}/StdAfx.cpp"
  "${XRCDB_DIR}/xrCDB.cpp"
  "${XRCDB_DIR}/xrCDB_box.cpp"
  "${XRCDB_DIR}/xrCDB_Collector.cpp"
  "${XRCDB_DIR}/xrCDB_frustum.cpp"
  "${XRCDB_DIR}/xrCDB_ray.cpp"
  "${XRCDB_DIR}/OPC_AABB.cpp"
  "${XRCDB_DIR}/OPC_AABBCollider.cpp"
  "${XRCDB_DIR}/OPC_AABBTree.cpp"
  "${XRCDB_DIR}/OPC_Collider.cpp"
  "${XRCDB_DIR}/OPC_Common.cpp"
  "${XRCDB_DIR}/OPC_Container.cpp"
  "${XRCDB_DIR}/OPC_Matrix3x3.cpp"
  "${XRCDB_DIR}/OPC_Matrix4x4.cpp"
  "${XRCDB_DIR}/OPC_Model.cpp"
  "${XRCDB_DIR}/OPC_OBB.cpp"
  "${XRCDB_DIR}/OPC_OBBCollider.cpp"
  "${XRCDB_DIR}/OPC_OptimizedTree.cpp"
  "${XRCDB_DIR}/OPC_Plane.cpp"
  "${XRCDB_DIR}/OPC_PlanesCollider.cpp"
  "${XRCDB_DIR}/OPC_Point.cpp"
  "${XRCDB_DIR}/OPC_Ray.cpp"
  "${XRCDB_DIR}/OPC_RayCollider.cpp"
  "${XRCDB_DIR}/OPC_SphereCollider.cpp"
  "${XRCDB_DIR}/OPC_TreeBuilders.cpp"
  "${XRCDB_DIR}/OPC_TreeCollider.cpp"
  "${XRCDB_DIR}/OPC_Triangle.cpp"
  "${XRCDB_DIR}/OPC_VolumeCollider.cpp"
  "${XRCDB_DIR}/Opcode.cpp"
)

add_library(xrCDB SHARED ${XRCDB_SOURCES})

set_target_properties(xrCDB PROPERTIES
  OUTPUT_NAME "xrCDB"
  # xrCDB.vcproj CharacterSet="2" is the VS2003 Multi-Byte setting.
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
  set_target_properties(xrCDB PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

# xrCDB.vcproj has no AdditionalIncludeDirectories. The project directory is
# still public because engine consumers include xrCDB headers directly.
target_include_directories(xrCDB
  PUBLIC
    "${XRCDB_DIR}"
  PRIVATE
    "${PROJECT_SOURCE_DIR}/cmake/compat/msvc"
)

target_compile_definitions(xrCDB
  PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    XRCDB_EXPORTS
    # xrCDB includes xrCore.h, so it needs the same modern MSVC compatibility
    # switches for legacy xrCore/STL headers even though they were implicit in
    # the original VC2003 environment.
    _FARQ=
    _HAS_AUTO_PTR_ETC=1
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    $<$<CONFIG:Mixed>:MIXED>
)

if(MSVC)
  target_compile_options(xrCDB
    PRIVATE
      /nologo
      /W4
      /GR
      /Gz
      /EHs-
      /EHa-
  )

  # Mirrors xrCDB.vcproj optimization/runtime choices where modern MSVC still
  # supports them; obsolete VC2003 settings such as /G7 and /Wp64 are omitted.
  target_compile_options(xrCDB PRIVATE
    $<$<CONFIG:Debug>:/Od /Zi /RTC1 /Gy /Oi->
    $<$<NOT:$<CONFIG:Debug>>:/O2 /Ob2 /Oi /Ot /GF /GS->
    $<$<CONFIG:Release>:/Oy /Zc:wchar_t->
  )

  target_link_options(xrCDB PRIVATE
    /nologo
    # StdAfx.cpp has the original #pragma comment(lib, "x:\\xrCore.lib").
    # CMake links the xrCore target directly, so ignore only that hardcoded
    # X: drive default library directive.
    /NODEFAULTLIB:x:\\xrCore.lib
    $<$<CONFIG:Debug>:/DEBUG /INCREMENTAL>
    $<$<NOT:$<CONFIG:Debug>>:/DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
  )
endif()

target_precompile_headers(xrCDB PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${XRCDB_DIR}/stdafx.h>")

target_link_libraries(xrCDB PRIVATE
  xrCore
  winmm
  $<$<CONFIG:Debug>:odbc32>
  $<$<CONFIG:Debug>:odbccp32>
)
