add_executable(MiracleQuickstart quickstart.cxx)
target_compile_features(MiracleQuickstart PRIVATE cxx_std_26)
set_target_properties(
  MiracleQuickstart
  PROPERTIES CXX_EXTENSIONS OFF
             CXX_STANDARD_REQUIRED ON
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD ON)
target_link_libraries(MiracleQuickstart PRIVATE Miracle::Miracle)

add_executable(MiracleDiagnosticExample diagnostic.cxx)
target_compile_features(MiracleDiagnosticExample PRIVATE cxx_std_26)
set_target_properties(
  MiracleDiagnosticExample
  PROPERTIES CXX_EXTENSIONS OFF
             CXX_STANDARD_REQUIRED ON
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD ON)
target_link_libraries(MiracleDiagnosticExample PRIVATE Miracle::Miracle)

add_executable(MiracleIterExample iter.cxx)
target_compile_features(MiracleIterExample PRIVATE cxx_std_26)
set_target_properties(
  MiracleIterExample
  PROPERTIES CXX_EXTENSIONS OFF
             CXX_STANDARD_REQUIRED ON
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD ON)
target_link_libraries(MiracleIterExample PRIVATE Miracle::Miracle)
