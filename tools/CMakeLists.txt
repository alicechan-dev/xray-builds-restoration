option(XRAY_BUILD_ARCHIVE_LIST "Build the read-only X-Ray archive listing diagnostic tool" ON)
option(BUILD_XR_UNPACK "Build the experimental xr_unpack archive unpacker scaffold" OFF)
option(BUILD_XR_ETOOLS "Build the experimental editor ETools SDK helper DLL" OFF)
option(BUILD_XR_ECORE "Build the experimental editor core SDK DLL" OFF)
option(BUILD_XR_EPROPS "Build the experimental editor properties SDK DLL" OFF)
option(BUILD_XR_LEVEL_EDITOR "Build the experimental historical LevelEditor shell" OFF)
option(BUILD_XR_LEVEL_OPTIONS "Build the experimental historical LevelOptions DLL shell" OFF)
option(BUILD_XR_SHADER_EDITOR "Build the experimental historical ShaderEditor shell" OFF)
option(BUILD_XR_PARTICLE_EDITOR "Build the experimental historical ParticleEditor shell" OFF)
option(BUILD_XR_ACTOR_EDITOR "Build the experimental historical ActorEditor shell" OFF)
option(BUILD_XR_LWO "Build the experimental historical LightWave object helper" OFF)
option(BUILD_XR_VCL_COMPAT_TESTS "Build isolated non-GUI VCL compatibility contract tests" OFF)
set(XR_LEGACY_DX_ROOT "" CACHE PATH "Optional DirectX SDK root used only by restored SDK/editor targets")
set(XR_ELPACK_ROOT "" CACHE PATH "Optional local ElPack root used only by restored SDK/editor targets")
set(XR_ALEXMX_ROOT "" CACHE PATH "Optional local AlexMX controls root used only by restored SDK/editor targets")
set(XR_MAGICFM_ROOT "" CACHE PATH "Optional local MagicFM SDK root used only by restored SDK/editor targets")
set(XR_BORLAND_ROOT "" CACHE PATH "Optional local Borland C++ Builder/VCL root used only by restored SDK/editor targets")

set(XR_EDITOR_SDK_ENABLED FALSE)
set(XR_EDITOR_SDK_TARGETS)
if(BUILD_XR_ECORE)
  set(XR_EDITOR_SDK_ENABLED TRUE)
  list(APPEND XR_EDITOR_SDK_TARGETS xrECore)
endif()
if(BUILD_XR_EPROPS)
  set(XR_EDITOR_SDK_ENABLED TRUE)
  list(APPEND XR_EDITOR_SDK_TARGETS xrEProps)
endif()
if(BUILD_XR_LEVEL_EDITOR)
  set(XR_EDITOR_SDK_ENABLED TRUE)
  list(APPEND XR_EDITOR_SDK_TARGETS LevelEditor xrECore xrEProps)
endif()
if(BUILD_XR_LEVEL_OPTIONS)
  set(XR_EDITOR_SDK_ENABLED TRUE)
  list(APPEND XR_EDITOR_SDK_TARGETS LevelOptions xrECore xrEProps)
endif()
if(BUILD_XR_SHADER_EDITOR)
  set(XR_EDITOR_SDK_ENABLED TRUE)
  list(APPEND XR_EDITOR_SDK_TARGETS ShaderEditor xrECore xrEProps)
endif()
if(BUILD_XR_PARTICLE_EDITOR)
  set(XR_EDITOR_SDK_ENABLED TRUE)
  list(APPEND XR_EDITOR_SDK_TARGETS ParticleEditor xrECore xrEProps)
endif()
if(BUILD_XR_ACTOR_EDITOR)
  set(XR_EDITOR_SDK_ENABLED TRUE)
  list(APPEND XR_EDITOR_SDK_TARGETS ActorEditor xrECore xrEProps)
endif()
list(REMOVE_DUPLICATES XR_EDITOR_SDK_TARGETS)

set(XR_EDITOR_ELPACK_INCLUDE_DIR "")
set(XR_EDITOR_ALEXMX_INCLUDE_DIR "")
set(XR_EDITOR_MAGICFM_INCLUDE_DIR "")
set(XR_EDITOR_BORLAND_INCLUDE_DIR "")
set(XR_EDITOR_BORLAND_VCL_INCLUDE_DIR "")
if(XR_EDITOR_SDK_ENABLED)
  list(JOIN XR_EDITOR_SDK_TARGETS ", " XR_EDITOR_SDK_TARGETS_TEXT)
  message(STATUS "SDK/editor dependency summary")
  message(STATUS "  Enabled targets: ${XR_EDITOR_SDK_TARGETS_TEXT}")

  if(XR_ELPACK_ROOT)
    foreach(candidate IN ITEMS
        "${XR_ELPACK_ROOT}/Code/Source"
        "${XR_ELPACK_ROOT}")
      if(EXISTS "${candidate}/ElTree.hpp")
        set(XR_EDITOR_ELPACK_INCLUDE_DIR "${candidate}")
        break()
      endif()
    endforeach()

    if(XR_EDITOR_ELPACK_INCLUDE_DIR)
      message(STATUS
        "  XR_ELPACK_ROOT: valid "
        "(headers: '${XR_EDITOR_ELPACK_INCLUDE_DIR}')")
    else()
      message(WARNING
        "SDK/editor dependency: XR_ELPACK_ROOT='${XR_ELPACK_ROOT}' is invalid. "
        "Expected Code/Source/ElTree.hpp or ElTree.hpp. Editor builds may stop "
        "at ElTree.hpp.")
    endif()
  else()
    message(WARNING
      "SDK/editor dependency: XR_ELPACK_ROOT is unset. ElPack is required by "
      "xrECore, xrEProps, LevelEditor, LevelOptions, ShaderEditor, and "
      "ParticleEditor, and ActorEditor; builds may stop at ElTree.hpp.")
  endif()

  if(XR_ALEXMX_ROOT)
    foreach(candidate IN ITEMS
        "${XR_ALEXMX_ROOT}"
        "${XR_ALEXMX_ROOT}/Include"
        "${XR_ALEXMX_ROOT}/include")
      if(EXISTS "${candidate}/ExtBtn.hpp" OR EXISTS "${candidate}/mxPlacemnt.hpp")
        set(XR_EDITOR_ALEXMX_INCLUDE_DIR "${candidate}")
        break()
      endif()
    endforeach()

    if(XR_EDITOR_ALEXMX_INCLUDE_DIR)
      message(STATUS
        "  XR_ALEXMX_ROOT: candidate "
        "(headers: '${XR_EDITOR_ALEXMX_INCLUDE_DIR}')")
    else()
      message(WARNING
        "SDK/editor dependency: XR_ALEXMX_ROOT='${XR_ALEXMX_ROOT}' is set, "
        "but expected AlexMX control headers such as ExtBtn.hpp or "
        "mxPlacemnt.hpp were not found.")
    endif()
  else()
    message(STATUS
      "  XR_ALEXMX_ROOT: unset "
      "(AlexMX controls remain unresolved for GUI editor probes)")
  endif()

  if(XR_MAGICFM_ROOT)
    foreach(candidate IN ITEMS
        "${XR_MAGICFM_ROOT}/Sdk/Include"
        "${XR_MAGICFM_ROOT}/SDK/Include"
        "${XR_MAGICFM_ROOT}/Include"
        "${XR_MAGICFM_ROOT}/include"
        "${XR_MAGICFM_ROOT}")
      if(IS_DIRECTORY "${candidate}")
        file(GLOB XR_MAGICFM_HEADER_CANDIDATES
          "${candidate}/*MagicFM*.h"
          "${candidate}/*MagicFM*.hpp"
          "${candidate}/*magicfm*.h"
          "${candidate}/*magicfm*.hpp")
        if(XR_MAGICFM_HEADER_CANDIDATES)
          set(XR_EDITOR_MAGICFM_INCLUDE_DIR "${candidate}")
          break()
        endif()
      endif()
    endforeach()

    if(XR_EDITOR_MAGICFM_INCLUDE_DIR)
      message(STATUS
        "  XR_MAGICFM_ROOT: candidate "
        "(headers: '${XR_EDITOR_MAGICFM_INCLUDE_DIR}')")
    else()
      message(WARNING
        "SDK/editor dependency: XR_MAGICFM_ROOT='${XR_MAGICFM_ROOT}' is set, "
        "but no MagicFM SDK headers were found. Existing MagicFM DLLs/libs "
        "alone are not enough for SDK GUI builds.")
    endif()
  else()
    message(STATUS
      "  XR_MAGICFM_ROOT: unset "
      "(MagicFM SDK headers remain unresolved)")
  endif()

  if(XR_BORLAND_ROOT)
    foreach(candidate IN ITEMS
        "${XR_BORLAND_ROOT}/include"
        "${XR_BORLAND_ROOT}/Include")
      if(IS_DIRECTORY "${candidate}")
        set(XR_EDITOR_BORLAND_INCLUDE_DIR "${candidate}")
        break()
      endif()
    endforeach()
    foreach(candidate IN ITEMS
        "${XR_BORLAND_ROOT}/include/vcl"
        "${XR_BORLAND_ROOT}/Include/vcl")
      if(IS_DIRECTORY "${candidate}")
        set(XR_EDITOR_BORLAND_VCL_INCLUDE_DIR "${candidate}")
        break()
      endif()
    endforeach()

    if(XR_EDITOR_BORLAND_INCLUDE_DIR AND XR_EDITOR_BORLAND_VCL_INCLUDE_DIR)
      message(STATUS
        "  XR_BORLAND_ROOT: candidate "
        "(include: '${XR_EDITOR_BORLAND_INCLUDE_DIR}', "
        "vcl: '${XR_EDITOR_BORLAND_VCL_INCLUDE_DIR}')")
    else()
      message(WARNING
        "SDK/editor dependency: XR_BORLAND_ROOT='${XR_BORLAND_ROOT}' is set, "
        "but expected include and include/vcl directories were not found. "
        "Borland/VCL GUI package restoration remains blocked.")
    endif()
  else()
    message(STATUS
      "  XR_BORLAND_ROOT: unset "
      "(true VCL GUI package restoration remains blocked)")
  endif()

  if(BUILD_XR_ECORE OR BUILD_XR_LEVEL_EDITOR OR BUILD_XR_LEVEL_OPTIONS OR BUILD_XR_SHADER_EDITOR OR BUILD_XR_PARTICLE_EDITOR OR BUILD_XR_ACTOR_EDITOR)
    if(XR_LEGACY_DX_ROOT)
      message(STATUS "  XR_LEGACY_DX_ROOT: '${XR_LEGACY_DX_ROOT}'")
    else()
      message(STATUS
        "  XR_LEGACY_DX_ROOT: unset "
        "(editor targets use the existing DirectX SDK fallback)")
    endif()
  endif()

  if(BUILD_XR_LEVEL_EDITOR OR BUILD_XR_LEVEL_OPTIONS OR BUILD_XR_SHADER_EDITOR OR BUILD_XR_PARTICLE_EDITOR OR BUILD_XR_ACTOR_EDITOR)
    if(XRAY_BOOST_ROOT)
      message(STATUS "  XRAY_BOOST_ROOT: '${XRAY_BOOST_ROOT}'")
    else()
      message(STATUS
        "  XRAY_BOOST_ROOT: unset "
        "(some editor projects historically requested Boost 1.30.x)")
    endif()
  endif()
endif()

if(XRAY_BUILD_ARCHIVE_LIST)
  add_subdirectory(xrArchiveList)
endif()

if(BUILD_XR_UNPACK)
  add_subdirectory(xr_unpack)
endif()

if(BUILD_XR_ETOOLS OR BUILD_XR_ECORE OR BUILD_XR_EPROPS OR BUILD_XR_LEVEL_EDITOR OR BUILD_XR_LEVEL_OPTIONS OR BUILD_XR_SHADER_EDITOR OR BUILD_XR_PARTICLE_EDITOR OR BUILD_XR_ACTOR_EDITOR)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/Tools/ETools" "${CMAKE_BINARY_DIR}/Editors/Tools/ETools")
endif()

if(BUILD_XR_EPROPS OR BUILD_XR_LEVEL_EDITOR OR BUILD_XR_LEVEL_OPTIONS OR BUILD_XR_SHADER_EDITOR OR BUILD_XR_PARTICLE_EDITOR OR BUILD_XR_ACTOR_EDITOR)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/xrEProps" "${CMAKE_BINARY_DIR}/Editors/xrEProps")
endif()

if(BUILD_XR_ECORE OR BUILD_XR_LEVEL_EDITOR OR BUILD_XR_LEVEL_OPTIONS OR BUILD_XR_SHADER_EDITOR OR BUILD_XR_PARTICLE_EDITOR OR BUILD_XR_ACTOR_EDITOR)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/ECore" "${CMAKE_BINARY_DIR}/Editors/ECore")
endif()

if(BUILD_XR_LEVEL_EDITOR)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/LevelEditor" "${CMAKE_BINARY_DIR}/Editors/LevelEditor")
endif()

if(BUILD_XR_LEVEL_OPTIONS)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/LevelOptions" "${CMAKE_BINARY_DIR}/Editors/LevelOptions")
endif()

if(BUILD_XR_SHADER_EDITOR)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/ShaderEditor" "${CMAKE_BINARY_DIR}/Editors/ShaderEditor")
endif()

if(BUILD_XR_PARTICLE_EDITOR)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/ParticleEditor" "${CMAKE_BINARY_DIR}/Editors/ParticleEditor")
endif()

if(BUILD_XR_ACTOR_EDITOR)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/ActorEditor" "${CMAKE_BINARY_DIR}/Editors/ActorEditor")
endif()

if(BUILD_XR_LWO)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/Tools/LWO" "${CMAKE_BINARY_DIR}/Editors/Tools/LWO")
endif()

if(BUILD_XR_VCL_COMPAT_TESTS)
  add_subdirectory("${PROJECT_SOURCE_DIR}/tests/compat/vcl" "${CMAKE_BINARY_DIR}/tests/compat/vcl")
endif()
