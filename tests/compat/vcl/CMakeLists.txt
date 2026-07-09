add_executable(vcl_ansi_string_contract
  ansi_string_contract.cpp
)

add_executable(vcl_msgdlg_type_contract
  msgdlg_type_contract.cpp
)

add_executable(vcl_msgdlg_buttons_contract
  msgdlg_buttons_contract.cpp
)

add_executable(vcl_shift_state_contract
  shift_state_contract.cpp
)

target_include_directories(vcl_ansi_string_contract PRIVATE
  "${PROJECT_SOURCE_DIR}/cmake/compat/vcl"
)

target_include_directories(vcl_msgdlg_type_contract PRIVATE
  "${PROJECT_SOURCE_DIR}/cmake/compat/vcl"
)

target_include_directories(vcl_msgdlg_buttons_contract PRIVATE
  "${PROJECT_SOURCE_DIR}/cmake/compat/vcl"
)

target_include_directories(vcl_shift_state_contract PRIVATE
  "${PROJECT_SOURCE_DIR}/cmake/compat/vcl"
)

set_target_properties(vcl_ansi_string_contract PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
)

set_target_properties(vcl_msgdlg_type_contract PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
)

set_target_properties(vcl_msgdlg_buttons_contract PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
)

set_target_properties(vcl_shift_state_contract PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
)
