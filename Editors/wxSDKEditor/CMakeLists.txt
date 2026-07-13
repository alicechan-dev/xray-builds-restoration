set(XR_WX_SDK_EDITOR_MODEL_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_app/EditorCommandHistory.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_app/EditorDocument.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_app/EditorModelCommand.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_model/EditorProperty.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_model/EditorPropertySet.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_model/EditorItemType.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_model/EditorSelectionModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_model/EditorTreeModel.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_model/EditorTreePathListImport.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_model/EditorTreeQuery.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_model/EditorTreeSnapshot.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_model/EditorTransform.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_view/EditorViewportController.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_view/EditorViewportState.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_view/EditorPreviewRenderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_view/EditorPreviewPicking.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_view/EditorMoveGizmo.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_view/EditorPreviewScene.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_view/EditorTreePreviewAdapter.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/editor_view/EditorViewportDrawList.cpp"
)

set(XR_WX_SDK_EDITOR_MODEL_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_app/EditorCommandHistory.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_app/EditorDocument.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_app/EditorModelCommand.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_app/IEditorCommand.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_model/EditorProperty.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_model/EditorPropertySet.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_model/EditorItemType.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_model/EditorSelectionModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_model/EditorTreeModel.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_model/EditorTreePathListImport.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_model/EditorTreeQuery.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_model/EditorTreeSnapshot.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_model/EditorTransform.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_view/EditorViewportController.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_view/EditorViewportState.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_view/IEditorViewportRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_view/EditorPreviewRenderer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_view/EditorPreviewPicking.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_view/EditorMoveGizmo.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_view/EditorPreviewScene.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_view/EditorTreePreviewAdapter.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/editor_view/EditorViewportDrawList.h"
)

if(BUILD_XR_WX_SDK_EDITOR_MODEL_TESTS)
  add_subdirectory(tests)
endif()

if(NOT BUILD_XR_WX_SDK_EDITOR)
  return()
endif()

set(XR_WX_SDK_EDITOR_SOURCES
  src/App.cpp
  ${XR_WX_SDK_EDITOR_MODEL_SOURCES}
  src/editor_app/EditorTreePresenter.cpp
  src/MainFrame.cpp
  src/wxDialogService.cpp
  src/wxEditorTree.cpp
  src/wxEditorViewport.cpp
  src/wxPropertyPanel.cpp
)

set(XR_WX_SDK_EDITOR_HEADERS
  include/editor_ui/IDialogService.h
  include/editor_ui/IEditorTree.h
  include/editor_ui/IPropertyPanel.h
  include/editor_app/EditorTreePresenter.h
  ${XR_WX_SDK_EDITOR_MODEL_HEADERS}
  src/MainFrame.h
  src/wxDialogService.h
  src/wxEditorTree.h
  src/wxEditorViewport.h
  src/wxPropertyPanel.h
)

set(XR_WX_SDK_EDITOR_LIBRARIES)
set(XR_WX_SDK_EDITOR_DISCOVERY "")

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(FATAL_ERROR
    "wxSDKEditor currently supports only Win32/x86. Configure the Visual "
    "Studio generator with '-A Win32' and use an x86 wxWidgets package.")
endif()

# vcpkg provides wxWidgets config packages on current installations. Fall back
# to CMake's standard module for system/package layouts that expose wx-config.
find_package(wxWidgets CONFIG QUIET COMPONENTS core base aui)
if(wxWidgets_FOUND AND TARGET wx::core AND TARGET wx::base AND TARGET wx::aui)
  set(XR_WX_SDK_EDITOR_LIBRARIES wx::core wx::base wx::aui)
  set(XR_WX_SDK_EDITOR_DISCOVERY "CONFIG package")
else()
  find_package(wxWidgets MODULE QUIET COMPONENTS core base aui)
  if(wxWidgets_FOUND)
    include("${wxWidgets_USE_FILE}")
    set(XR_WX_SDK_EDITOR_LIBRARIES ${wxWidgets_LIBRARIES})
    set(XR_WX_SDK_EDITOR_DISCOVERY "FindwxWidgets module")
  endif()
endif()

if(NOT XR_WX_SDK_EDITOR_LIBRARIES)
  message(FATAL_ERROR
    "wxSDKEditor requires wxWidgets for Win32/x86. Install it outside the "
    "repository with 'vcpkg install wxwidgets:x86-windows', then configure "
    "with '-DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake'. "
    "Do not use the x64-windows triplet for this Win32 target.")
endif()

message(STATUS "wxSDKEditor wxWidgets discovery: ${XR_WX_SDK_EDITOR_DISCOVERY}")

add_executable(wxSDKEditor WIN32
  ${XR_WX_SDK_EDITOR_SOURCES}
  ${XR_WX_SDK_EDITOR_HEADERS}
)

target_include_directories(wxSDKEditor PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(wxSDKEditor PRIVATE
  UNICODE
  _UNICODE
  WXUSINGDLL
)

target_link_libraries(wxSDKEditor PRIVATE
  ${XR_WX_SDK_EDITOR_LIBRARIES}
)

set_target_properties(wxSDKEditor PROPERTIES
  OUTPUT_NAME "wxSDKEditor"
  VS_GLOBAL_CharacterSet "Unicode"
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(wxSDKEditor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

if(MSVC)
  # The historical root removes /EHsc for runtime compatibility. wxWidgets is
  # built with standard C++ exceptions and RTTI, so restore them locally.
  target_compile_options(wxSDKEditor PRIVATE /W4 /permissive- /EHsc /GR)
endif()
