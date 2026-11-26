foreach(
  name IN
  ITEMS pointer-sort
        rvalue-range-flatten
        namespace-cache-visibility
        namespace-functions-visibility
        annotation-redeclaration-visibility
        annotation-typed-redeclaration-visibility
        late-class-members
        predicate-participation
        meta-function-participation
        api-participation
        formatter-boundaries
        query-edges
        stable-controls)
  add_executable("MiracleMetaHardening${name}" "${name}.cpp")
  target_compile_features("MiracleMetaHardening${name}" PRIVATE cxx_std_26)
  set_target_properties(
    "MiracleMetaHardening${name}"
    PROPERTIES CXX_EXTENSIONS OFF
               CXX_STANDARD_REQUIRED ON
               CXX_SCAN_FOR_MODULES ON
               CXX_MODULE_STD ON)
  target_link_libraries("MiracleMetaHardening${name}" PRIVATE Miracle::Miracle)
  add_test(NAME "Miracle.Meta.Hardening.${name}"
           COMMAND "MiracleMetaHardening${name}")
endforeach()

add_executable(MiracleMetaHardeningCrossTU cross-tu-a.cpp cross-tu-b.cpp)
target_compile_features(MiracleMetaHardeningCrossTU PRIVATE cxx_std_26)
set_target_properties(
  MiracleMetaHardeningCrossTU
  PROPERTIES CXX_EXTENSIONS OFF
             CXX_STANDARD_REQUIRED ON
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD ON)
target_link_libraries(MiracleMetaHardeningCrossTU PRIVATE Miracle::Miracle)
add_test(NAME Miracle.Meta.Hardening.CrossTU
         COMMAND MiracleMetaHardeningCrossTU)

# Guard the measured fresh-snapshot constexpr budget without raising compiler
# limits.
set(SNAPSHOT_FIELDS "")
foreach(index RANGE 0 511)
  string(APPEND SNAPSHOT_FIELDS "  int field${index};\n")
endforeach()
configure_file(snapshot-stress.cpp.in
               "${CMAKE_CURRENT_BINARY_DIR}/snapshot-stress.cpp" @ONLY)
add_executable(MiracleMetaSnapshotStress
               "${CMAKE_CURRENT_BINARY_DIR}/snapshot-stress.cpp")
target_compile_features(MiracleMetaSnapshotStress PRIVATE cxx_std_26)
set_target_properties(
  MiracleMetaSnapshotStress
  PROPERTIES CXX_EXTENSIONS OFF
             CXX_STANDARD_REQUIRED ON
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD ON)
target_link_libraries(MiracleMetaSnapshotStress PRIVATE Miracle::Miracle)
add_test(NAME Miracle.Meta.Hardening.SnapshotStress
         COMMAND MiracleMetaSnapshotStress)
