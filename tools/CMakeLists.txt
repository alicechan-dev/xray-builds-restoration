option(XRAY_BUILD_ARCHIVE_LIST "Build the read-only X-Ray archive listing diagnostic tool" ON)
option(BUILD_XR_UNPACK "Build the experimental xr_unpack archive unpacker scaffold" OFF)
option(BUILD_XR_ETOOLS "Build the experimental editor ETools SDK helper DLL" OFF)

if(XRAY_BUILD_ARCHIVE_LIST)
  add_subdirectory(xrArchiveList)
endif()

if(BUILD_XR_UNPACK)
  add_subdirectory(xr_unpack)
endif()

if(BUILD_XR_ETOOLS)
  add_subdirectory("${PROJECT_SOURCE_DIR}/Editors/Tools/ETools" "${CMAKE_BINARY_DIR}/Editors/Tools/ETools")
endif()
