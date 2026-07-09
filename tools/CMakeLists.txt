option(XRAY_BUILD_ARCHIVE_LIST "Build the read-only X-Ray archive listing diagnostic tool" ON)
option(BUILD_XR_UNPACK "Build the experimental xr_unpack archive unpacker scaffold" OFF)
option(BUILD_XR_ETOOLS "Build the experimental editor ETools SDK helper DLL" OFF)
option(BUILD_XR_ECORE "Build the experimental editor core SDK DLL" OFF)
option(BUILD_XR_EPROPS "Build the experimental editor properties SDK DLL" OFF)
option(BUILD_XR_LEVEL_EDITOR "Build the experimental historical LevelEditor shell" OFF)
option(BUILD_XR_VCL_COMPAT_TESTS "Build isolated non-GUI VCL compatibility contract tests" OFF)
set(XR_LEGACY_DX_ROOT "" CACHE PATH "Optional DirectX SDK root used only by restored SDK/editor targets")
set(XR_ELPACK_ROOT "" CACHE PATH "Optional local ElPack root used only by restored SDK/editor targets")

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
list(REMOVE_DUPLICATES XR_EDITOR_SDK_TARGETS)

set(XR_EDITOR_ELPACK_INCLUDE_DIR "")
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
      "xrECore, xrEProps, and LevelEditor; builds may stop at ElTree.hpp.")
  endif()

  if(BUILD_XR_ECORE OR BUILD_XR_LEVEL_EDITOR)
    if(XR_LEGACY_DX_ROOT)
      message(STATUS "  XR_LEGACY_DX_ROOT: '${XR_LEGACY_DX_ROOT}'")
    else()
      message(STATUS
        "  XR_LEGACY_DX_ROOT: unset "
        "(editor targets use the existing DirectX SDK fallback)")
    endif()
  endif()

  if(BUILD_XR_LEVEL_EDITOR)
    if(XRAY_BOOST_ROOT)
      message(STATUS "  XRAY_BOOST_ROOT: '${XRAY_BOOST_ROOT}'")
    else()
      message(STATUS
        "  XRAY_BOOST_ROOT: unset "
        "(LevelEditor historically requested Boost 1.30.x)")
    endif()
  endif()
endif()

if(XRAY_BUILD_ARCHIVE_LIST)
  add_subdirectory(xrArchiveList)
endif()

if(BUILD_XR_UNPACK)
  add_subdirectory(xr_unpack)
endif()

if(BUILD_XR_ETOOLS OR BUILD_XR_ECORE OR BUILD_XR_EPROPS OR BUILD_XR_LEVEL_EDITOR)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/Tools/ETools" "${CMAKE_BINARY_DIR}/Editors/Tools/ETools")
endif()

if(BUILD_XR_EPROPS OR BUILD_XR_LEVEL_EDITOR)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/xrEProps" "${CMAKE_BINARY_DIR}/Editors/xrEProps")
endif()

if(BUILD_XR_ECORE OR BUILD_XR_LEVEL_EDITOR)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/ECore" "${CMAKE_BINARY_DIR}/Editors/ECore")
endif()

if(BUILD_XR_LEVEL_EDITOR)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/LevelEditor" "${CMAKE_BINARY_DIR}/Editors/LevelEditor")
endif()

if(BUILD_XR_VCL_COMPAT_TESTS)
  add_subdirectory("${PROJECT_SOURCE_DIR}/tests/compat/vcl" "${CMAKE_BINARY_DIR}/tests/compat/vcl")
endif()
