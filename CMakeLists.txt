cmake_minimum_required(VERSION 3.24)

project(XRayBuild2571Candidate LANGUAGES C CXX RC)

set(XRAY_OUTPUT_ROOT "${CMAKE_BINARY_DIR}" CACHE PATH "Root for generated build outputs")
set(XRAY_RUNTIME_DIR "${XRAY_OUTPUT_ROOT}/bin" CACHE PATH "Directory for executables and runtime DLLs")
set(XRAY_LIBRARY_DIR "${XRAY_OUTPUT_ROOT}/lib" CACHE PATH "Directory for static and import libraries")
set(XRAY_PDB_DIR "${XRAY_OUTPUT_ROOT}/pdb" CACHE PATH "Directory for debug symbols")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${XRAY_LIBRARY_DIR}")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${XRAY_PDB_DIR}")

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed ReleaseDebug DebugNoExceptions)
  string(TOUPPER "${config}" config_upper)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${XRAY_LIBRARY_DIR}")
  set(CMAKE_PDB_OUTPUT_DIRECTORY_${config_upper} "${XRAY_PDB_DIR}")
endforeach()

if(NOT WIN32)
  message(WARNING "The initial 2571 candidate restoration targets Windows Win32/x86. Non-Windows builds are future portability work.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(WARNING "The checked-in legacy projects target Win32. Configure with a 32-bit generator, for example: -A Win32.")
endif()

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

include(cmake/XRay2571Dependencies.cmake)

add_subdirectory(xrCore)

set(XRAY2571_RESTORATION_BUILD_ORDER
  xrCore
  xrCDB
  xrLUA
  xrLUA_JIT
  xrXMLParser
  xrParticles
  xrNetServer
  xrSound
  xrSE_Factory
  ode
  XR_3DA
  xrCPU_Pipe
  xrRender_R1
  xrRender_R2
  xrGame
  xrCompress
  xrFS
  xrLC
  xrAI
  xrQSlim
)

add_custom_target(xray2571_build_order
  COMMAND ${CMAKE_COMMAND} -E echo "2571 candidate restoration build order:"
  COMMAND ${CMAKE_COMMAND} -E echo "${XRAY2571_RESTORATION_BUILD_ORDER}"
  COMMENT "Printing the documented initial target restoration order"
)

message(STATUS "2571 candidate CMake scaffold configured.")
message(STATUS "Enabled restored targets: xrCore. Restore additional targets incrementally from the legacy project files.")
