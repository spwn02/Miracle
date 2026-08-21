add_executable(MiracleQuickstart quickstart.cxx)

target_compile_features(MiracleQuickstart PRIVATE cxx_std_26)

set_target_properties(
  MiracleQuickstart
  PROPERTIES CXX_EXTENSIONS OFF
             CXX_STANDARD_REQUIRED ON
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD ON)

target_link_libraries(MiracleQuickstart PRIVATE Miracle::Miracle)
