add_executable(wxSDKEditorModelTests
  EditorTreeModelTests.cpp
  EditorDocumentTests.cpp
  EditorViewportControllerTests.cpp
  EditorPreviewSceneTests.cpp
  EditorTreePresenterTests.cpp
  EditorToolControllerTests.cpp
  EditorMetadataTests.cpp
  EditorSceneProbeTests.cpp
  EditorSceneCompressionTests.cpp
  EditorHistoricalSceneDocumentTests.cpp
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
