add_executable(wxSDKEditorModelTests
  EditorTreeModelTests.cpp
  EditorDocumentTests.cpp
  EditorViewportControllerTests.cpp
  EditorPreviewSceneTests.cpp
  EditorTreePresenterTests.cpp
  EditorToolControllerTests.cpp
  EditorMetadataTests.cpp
  EditorObjectLibraryTests.cpp
  EditorStaticMeshTests.cpp
  EditorSceneProbeTests.cpp
  EditorSceneCompressionTests.cpp
  EditorHistoricalSceneDocumentTests.cpp
  EditorHistoricalObjectBodyDecoderTests.cpp
  EditorHistoricalSceneConversionTests.cpp
  EditorHistoricalOriginIntegrityTests.cpp
  "${CMAKE_CURRENT_SOURCE_DIR}/../src/editor_app/EditorTreePresenter.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/editor_app/EditorTreePresenter.h"
  ${XR_WX_SDK_EDITOR_MODEL_SOURCES}
  ${XR_WX_SDK_EDITOR_MODEL_HEADERS}
)

target_include_directories(wxSDKEditorModelTests PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

set_target_properties(wxSDKEditorModelTests PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
  RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}"
  PDB_OUTPUT_DIRECTORY     "${XRAY_OUTPUT_ROOT}/pdb"
)

foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
  string(TOUPPER "${config}" config_upper)
  set_target_properties(wxSDKEditorModelTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}"
    PDB_OUTPUT_DIRECTORY_${config_upper}     "${XRAY_OUTPUT_ROOT}/pdb"
  )
endforeach()

if(MSVC)
  target_compile_options(wxSDKEditorModelTests PRIVATE /W4 /permissive- /EHsc /GR)
endif()

if(BUILD_XR_WX_SDK_EDITOR AND WIN32)
  add_executable(wxSDKEditorD3D11Tests
    EditorD3D11IntegrationTests.cpp)
  target_link_libraries(wxSDKEditorD3D11Tests PRIVATE d3d11 dxgi d3dcompiler)
  set_target_properties(wxSDKEditorD3D11Tests PROPERTIES
    CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO
    RUNTIME_OUTPUT_DIRECTORY "${XRAY_RUNTIME_DIR}")
  foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel Mixed)
    string(TOUPPER "${config}" config_upper)
    set_target_properties(wxSDKEditorD3D11Tests PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${XRAY_RUNTIME_DIR}")
  endforeach()
  if(MSVC)
    target_compile_options(wxSDKEditorD3D11Tests PRIVATE /W4 /permissive- /EHsc)
  endif()
endif()
