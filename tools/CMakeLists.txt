option(XRAY_BUILD_ARCHIVE_LIST "Build the read-only X-Ray archive listing diagnostic tool" ON)
option(BUILD_XR_UNPACK "Build the experimental xr_unpack archive unpacker scaffold" OFF)

if(XRAY_BUILD_ARCHIVE_LIST)
  add_subdirectory(xrArchiveList)
endif()

if(BUILD_XR_UNPACK)
  add_subdirectory(xr_unpack)
endif()
