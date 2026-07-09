add_executable(vcl_ansi_string_contract
  ansi_string_contract.cpp
)

target_include_directories(vcl_ansi_string_contract PRIVATE
  "${PROJECT_SOURCE_DIR}/cmake/compat/vcl"
)

set_target_properties(vcl_ansi_string_contract PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
)
