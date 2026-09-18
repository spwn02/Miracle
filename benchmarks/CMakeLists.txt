add_executable(MiracleIterBenchmark iter.cxx)
add_executable(MiracleIterStress iter-stress.cxx)

foreach(target IN ITEMS MiracleIterBenchmark MiracleIterStress)
  target_compile_features(${target} PRIVATE cxx_std_26)
  set_target_properties(
    ${target}
    PROPERTIES CXX_EXTENSIONS OFF
               CXX_STANDARD_REQUIRED ON
               CXX_SCAN_FOR_MODULES ON
               CXX_MODULE_STD ON)
  target_link_libraries(${target} PRIVATE Miracle::Miracle)
endforeach()

if(MIRACLE_BUILD_TESTS)
  add_test(NAME MiracleIterStress COMMAND MiracleIterStress)
endif()
