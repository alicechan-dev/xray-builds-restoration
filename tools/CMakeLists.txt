option(XRAY_BUILD_ARCHIVE_LIST "Build the read-only X-Ray archive listing diagnostic tool" ON)
option(BUILD_XR_UNPACK "Build the experimental xr_unpack archive unpacker scaffold" OFF)
option(BUILD_XR_ETOOLS "Build the experimental editor ETools SDK helper DLL" OFF)
option(BUILD_XR_ECORE "Build the experimental editor core SDK DLL" OFF)
option(BUILD_XR_EPROPS "Build the experimental editor properties SDK DLL" OFF)
option(BUILD_XR_VCL_COMPAT_TESTS "Build isolated non-GUI VCL compatibility contract tests" OFF)
set(XR_LEGACY_DX_ROOT "" CACHE PATH "Optional DirectX SDK root used only by restored SDK/editor targets")
set(XR_ELPACK_ROOT "" CACHE PATH "Optional local ElPack root used only by restored SDK/editor targets")

if(XRAY_BUILD_ARCHIVE_LIST)
  add_subdirectory(xrArchiveList)
endif()

if(BUILD_XR_UNPACK)
  add_subdirectory(xr_unpack)
endif()

if(BUILD_XR_ETOOLS OR BUILD_XR_ECORE OR BUILD_XR_EPROPS)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/Tools/ETools" "${CMAKE_BINARY_DIR}/Editors/Tools/ETools")
endif()

if(BUILD_XR_EPROPS)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/xrEProps" "${CMAKE_BINARY_DIR}/Editors/xrEProps")
endif()

if(BUILD_XR_ECORE)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/ECore" "${CMAKE_BINARY_DIR}/Editors/ECore")
endif()

if(BUILD_XR_VCL_COMPAT_TESTS)
  add_subdirectory("${PROJECT_SOURCE_DIR}/tests/compat/vcl" "${CMAKE_BINARY_DIR}/tests/compat/vcl")
endif()
