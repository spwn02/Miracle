if(NOT TARGET Switch::Switch)
  find_package(Switch CONFIG QUIET)
endif()

if(NOT TARGET Switch::Switch)
  include(FetchContent)

  FetchContent_Declare(
    Switch
    GIT_REPOSITORY https://github.com/spwn02/Switch.git
    GIT_TAG 341ccd8b08196a27108f5a583160d4d20346a5f7
    GIT_SHALLOW FALSE
    PATCH_COMMAND "${CMAKE_COMMAND}" "-DSOURCE_DIR=<SOURCE_DIR>" -P
                  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/patch-switch-panic.cmake")
  FetchContent_MakeAvailable(Switch)
endif()

if(NOT TARGET Switch::Switch)
  message(
    FATAL_ERROR
      "MIRACLE_BUILD_TESTS requires Switch::Switch"
      "Provide an existing target, an installed Switch package, "
      "FETCHCONTENT_SOURCE_DIR_SWITCH, or the standalone Switch repository.")
endif()

add_executable(
  MiracleTests
  main.cxx
  bitflags.cxx
  debug.cxx
  diagnostic.cxx
  error.cxx
  fs.cxx
  range.cxx
  iter.cxx
  meta.cxx
  reflection.cxx
  static-string.cxx
  feature.cxx)

target_compile_features(MiracleTests PRIVATE cxx_std_26)

set_target_properties(
  MiracleTests
  PROPERTIES CXX_EXTENSIONS OFF
             CXX_STANDARD_REQUIRED ON
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD ON)

target_link_libraries(MiracleTests PRIVATE Miracle::Miracle Switch::Switch)

# Verify that the Phase-5 narrow Meta module is independently consumable; this
# probe deliberately does not import the Miracle umbrella or std module.
add_executable(MiracleMetaImportProbe meta-import-probe.cxx)
target_compile_features(MiracleMetaImportProbe PRIVATE cxx_std_26)
set_target_properties(
  MiracleMetaImportProbe
  PROPERTIES CXX_EXTENSIONS OFF
             CXX_STANDARD_REQUIRED ON
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD ON)
target_link_libraries(MiracleMetaImportProbe PRIVATE Miracle::Miracle)
add_test(NAME Miracle.Meta.Import COMMAND MiracleMetaImportProbe)

# The filesystem tests intentionally write only below the build tree.
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests")

add_test(NAME Miracle.Tests COMMAND MiracleTests)
set_tests_properties(Miracle.Tests PROPERTIES WORKING_DIRECTORY
                                              "${CMAKE_CURRENT_BINARY_DIR}")

# Panic is terminal by contract, so each overload runs in its own subprocess.
add_executable(MiraclePanicProbe panic-probe.cxx)
target_compile_features(MiraclePanicProbe PRIVATE cxx_std_26)
set_target_properties(
  MiraclePanicProbe
  PROPERTIES CXX_EXTENSIONS OFF
             CXX_STANDARD_REQUIRED ON
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD ON)
target_link_libraries(MiraclePanicProbe PRIVATE Miracle::Miracle)

function(miracle_add_panic_test name argument expected)
  add_test(
    NAME "Miracle.Panic.${name}"
    COMMAND
      "${CMAKE_COMMAND}" "-DPROGRAM=$<TARGET_FILE:MiraclePanicProbe>"
      "-DARGUMENT=${argument}" "-DEXPECTED=${expected}" -P
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect-process.cmake")
endfunction()

miracle_add_panic_test(String 0 "error: string panic")
miracle_add_panic_test(Error 1 "error: error panic")
miracle_add_panic_test(Diagnostic 2 "error[P9]: diagnostic panic")
miracle_add_panic_test(Trace 3 "stacktrace:")
miracle_add_panic_test(RenderFallback 4 "panic: fallback panic")

# Contract adaptation is exercised through a test-local standardized handler;
# Miracle itself intentionally does not install the process-global handler yet.
add_executable(MiracleContractProbe contract-probe.cxx)
target_compile_features(MiracleContractProbe PRIVATE cxx_std_26)
set_target_properties(
  MiracleContractProbe
  PROPERTIES CXX_EXTENSIONS OFF
             CXX_STANDARD_REQUIRED ON
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD ON)
target_link_libraries(MiracleContractProbe PRIVATE Miracle::Miracle)
function(miracle_add_contract_test name argument expected)
  add_test(
    NAME "Miracle.Contracts.${name}"
    COMMAND
      "${CMAKE_COMMAND}" "-DPROGRAM=$<TARGET_FILE:MiracleContractProbe>"
      "-DARGUMENT=${argument}" "-DEXPECTED=${expected}" "-DEXPECTED_EXIT=42" -P
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect-process.cmake")
endfunction()

miracle_add_contract_test(Precondition 0 "error[CTR001]: value > 0")
miracle_add_contract_test(Postcondition 1 "error[CTR002]: result : result > 0")
miracle_add_contract_test(Assertion 2 "error[CTR003]: value > 0")

# Build-bridge tests exercise feature registration/resolution without requiring
# a compiler invocation, including negative cycle/conflict space.
add_test(NAME Miracle.Features.CMake
         COMMAND "${CMAKE_COMMAND}" -P
                 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/feature-engine.cmake")
add_test(NAME Miracle.Features.CMakeEmpty
         COMMAND "${CMAKE_COMMAND}" -P
                 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/feature-empty.cmake")
add_test(
  NAME Miracle.Features.Configuration
  COMMAND
    "${CMAKE_COMMAND}"
    "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/feature-configuration" -P
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/feature-configuration.cmake")
# Negative CMake tests verify the intended Miracle diagnostic instead of merely
# accepting any non-zero configure result. This prevents syntax errors or test
# harness regressions from masquerading as successful negative-space coverage.
function(miracle_add_cmake_failure_test name script expected)
  add_test(
    NAME "Miracle.Features.CMake${name}"
    COMMAND
      "${CMAKE_COMMAND}" "-DSCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/cmake/${script}"
      "-DEXPECTED=${expected}" -P
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect-cmake-failure.cmake")
endfunction()

miracle_add_cmake_failure_test(
  Cycle feature-cycle.cmake
  "Miracle feature dependency cycle: a --> b --> c --> a")
miracle_add_cmake_failure_test(Conflict feature-conflict.cmake
                               "Miracle feature 'a' conflicts with 'b'")
miracle_add_cmake_failure_test(Missing feature-missing.cmake
                               "Miracle feature 'absent' is not registered")
miracle_add_cmake_failure_test(
  Capability feature-capability.cmake
  "Miracle feature 'reflection' requires unavailable capability")
miracle_add_cmake_failure_test(IdCollision feature-id-collision.cmake
                               "map to the same build")

# Compile-fail targets are excluded from ordinary builds. CTest explicitly asks
# the build tool to compile each target and then verifies that the intended
# Miracle diagnostic, not an unrelated compiler failure, caused rejection.
function(miracle_add_compile_fail_test name source expected)
  set(target "MiracleCompileFail${name}")
  add_executable("${target}" EXCLUDE_FROM_ALL "${source}")
  target_compile_features("${target}" PRIVATE cxx_std_26)
  set_target_properties(
    "${target}"
    PROPERTIES CXX_EXTENSIONS OFF
               CXX_STANDARD_REQUIRED ON
               CXX_SCAN_FOR_MODULES ON
               CXX_MODULE_STD ON)
  target_link_libraries("${target}" PRIVATE Miracle::Miracle)

  add_test(
    NAME "Miracle.Features.CompileFail.${name}"
    COMMAND
      "${CMAKE_COMMAND}" "-DBUILD_DIR=${CMAKE_BINARY_DIR}" "-DTARGET=${target}"
      "-DEXPECTED=${expected}" -P
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect-build-failure.cmake")
  set_tests_properties("Miracle.Features.CompileFail.${name}"
                       PROPERTIES RESOURCE_LOCK miracle_compile_fail)
endfunction()

miracle_add_compile_fail_test(Disabled compile-fail/feature-disabled.cxx
                              "feature `tracing` is not enabled")
miracle_add_compile_fail_test(
  Cycle compile-fail/feature-cycle.cxx
  "feature dependency cycle: cycle-a --> cycle-b --> cycle-c --> cycle-a")
miracle_add_compile_fail_test(
  Conflict compile-fail/feature-conflict.cxx
  "feature conflict: `first` conflicts with `second`")
miracle_add_compile_fail_test(
  DiagnosticNegative compile-fail/diagnostic-negative.cxx
  "because 'NegativeCode' does not satisfy 'DiagnosticCode'")
miracle_add_compile_fail_test(
  DiagnosticAlignWithoutPrefix compile-fail/diagnostic-align-without-prefix.cxx
  "because 'InvalidAlignment' does not satisfy 'DiagnosticCode'")

miracle_add_compile_fail_test(
  IterAdaptiveInvocation compile-fail/iter-adaptive-invocation.cxx
  "Miracle adaptive invocation requires a callable accepting the item directly")

miracle_add_compile_fail_test(
  MetaRequireName compile-fail/meta-require-name.cxx
  "requireName() requires a reflection with an identifier")
miracle_add_compile_fail_test(
  MetaEnumeratorsNonEnum compile-fail/meta-enumerators-non-enum.cxx
  "enumerators() requires an enum reflection")
