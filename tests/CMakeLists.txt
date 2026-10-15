if(NOT TARGET Switch::Switch)
  find_package(Switch CONFIG QUIET)
endif()

if(NOT TARGET Switch::Switch)
  include(FetchContent)

  FetchContent_Declare(
    Switch
    GIT_REPOSITORY https://github.com/spwn02/Switch.git
    GIT_TAG d5587e5445d13d863c27ab7b5eef274db6523276
    GIT_SHALLOW FALSE)
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
  meta/semantics/meta.cxx
  meta/query/sources.cxx
  meta/query/barriers.cxx
  meta/query/lifetimes.cxx
  meta/query/interop.cxx
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

# Verify that the narrow Meta module is independently consumable; this probe
# deliberately does not import the Miracle umbrella or std module.
add_executable(MiracleMetaImportProbe import.cxx)
target_compile_features(MiracleMetaImportProbe PRIVATE cxx_std_26)
set_target_properties(
  MiracleMetaImportProbe
  PROPERTIES CXX_EXTENSIONS OFF
             CXX_STANDARD_REQUIRED ON
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD ON)
target_link_libraries(MiracleMetaImportProbe PRIVATE Miracle::Miracle)
add_test(NAME Miracle.Meta.Import COMMAND MiracleMetaImportProbe)

# Compile-time parity against the pinned C++26 <meta> surface. Parity is split
# by responsibility so a regression in one reflection family is independently
# attributable instead of hiding inside one monolithic TU.
function(miracle_add_meta_parity_test name source)
  set(target "MiracleMetaParity${name}")
  add_executable("${target}" "meta/parity/${source}")
  target_compile_features("${target}" PRIVATE cxx_std_26)
  set_target_properties(
    "${target}"
    PROPERTIES CXX_EXTENSIONS OFF
               CXX_STANDARD_REQUIRED ON
               CXX_SCAN_FOR_MODULES ON
               CXX_MODULE_STD ON)
  target_link_libraries("${target}" PRIVATE Miracle::Miracle)
  add_test(NAME "Miracle.Meta.Parity.${name}" COMMAND "${target}")
endfunction()

miracle_add_meta_parity_test(Types types.cxx)
miracle_add_meta_parity_test(Storage storage.cxx)
miracle_add_meta_parity_test(Names names.cxx)
miracle_add_meta_parity_test(Operators operators.cxx)
miracle_add_meta_parity_test(Layout layout.cxx)
miracle_add_meta_parity_test(Access access.cxx)
miracle_add_meta_parity_test(Members members.cxx)
miracle_add_meta_parity_test(Annotations annotations.cxx)
miracle_add_meta_parity_test(Predicates predicates.cxx)
miracle_add_meta_parity_test(TypeQueries type-queries.cxx)
miracle_add_meta_parity_test(TypeTransforms type-transforms.cxx)
miracle_add_meta_parity_test(Values values.cxx)
miracle_add_meta_parity_test(Substitution substitution.cxx)
miracle_add_meta_parity_test(Generation generation.cxx)
miracle_add_meta_parity_test(Reflect reflect.cxx)
miracle_add_meta_parity_test(Surface surface.cxx)
miracle_add_meta_parity_test(Comprehensive std.cxx)

add_subdirectory(meta/hardening)

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
# The pinned reference toolchain ignores contract predicates in Release and
# MinSizeRel. Match the test expectation to the final semantic flag for the
# active single-config build instead of requiring diagnostics when checks are
# off.
set(miracle_contract_flags "${CMAKE_CXX_FLAGS}")
if(CMAKE_BUILD_TYPE)
  string(TOUPPER "${CMAKE_BUILD_TYPE}" miracle_contract_config)
  set(miracle_contract_config_flags
      "CMAKE_CXX_FLAGS_${miracle_contract_config}")
  string(APPEND miracle_contract_flags " ${${miracle_contract_config_flags}}")
endif()
string(REGEX MATCHALL "-fcontract-evaluation-semantic=[A-Za-z_]+"
             miracle_contract_semantic_flags "${miracle_contract_flags}")
set(miracle_contracts_ignored FALSE)
if(miracle_contract_semantic_flags)
  list(GET miracle_contract_semantic_flags -1
       miracle_contract_last_semantic_flag)
  if(miracle_contract_last_semantic_flag STREQUAL
     "-fcontract-evaluation-semantic=ignore")
    set(miracle_contracts_ignored TRUE)
  endif()
endif()

function(miracle_add_contract_test name argument expected)
  set(expected_exit 42)
  set(expect_empty_stderr FALSE)
  if(miracle_contracts_ignored)
    set(expected_exit 0)
    set(expected "")
    set(expect_empty_stderr TRUE)
  endif()

  set(command
      "${CMAKE_COMMAND}" "-DPROGRAM=$<TARGET_FILE:MiracleContractProbe>"
      "-DARGUMENT=${argument}" "-DEXPECTED=${expected}"
      "-DEXPECTED_EXIT=${expected_exit}")
  if(expect_empty_stderr)
    list(APPEND command "-DEXPECT_EMPTY_STDERR=ON")
  endif()
  list(APPEND command -P
       "${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect-process.cmake")
  add_test(NAME "Miracle.Contracts.${name}" COMMAND ${command})
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

function(miracle_add_meta_compile_fail_test name source expected)
  set(target "MiracleMetaCompileFail${name}")
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
    NAME "Miracle.Meta.CompileFail.${name}"
    COMMAND
      "${CMAKE_COMMAND}" "-DBUILD_DIR=${CMAKE_BINARY_DIR}" "-DTARGET=${target}"
      "-DEXPECTED=${expected}" -P
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect-build-failure.cmake")
  set_tests_properties("Miracle.Meta.CompileFail.${name}"
                       PROPERTIES RESOURCE_LOCK miracle_compile_fail)
endfunction()

miracle_add_meta_compile_fail_test(
  QueryTemporaryArray meta/query/compile-fail/temporary-array.cxx
  "Query cannot borrow a temporary array")
miracle_add_meta_compile_fail_test(
  QueryBorrowedLocalEscape meta/query/compile-fail/borrowed-local-escape.cxx
  "lifetime has ended")
miracle_add_meta_compile_fail_test(
  QueryReferenceMaterialize meta/query/compile-fail/reference-materialize.cxx
  "Query::materialize() requires a copy-constructible object value")
miracle_add_meta_compile_fail_test(
  QueryNonStructuralBarrier meta/query/compile-fail/non-structural-barrier.cxx
  "Query persistence barrier requires a structurally promotable stored element")
miracle_add_meta_compile_fail_test(
  QueryMoveOnlyBarrier meta/query/compile-fail/move-only-barrier.cxx
  "Query persistence barrier requires a copy-constructible owning logical value"
)
miracle_add_meta_compile_fail_test(
  QueryLocalReferenceBarrier
  meta/query/compile-fail/local-reference-barrier.cxx
  "provided value cannot be represented by a reflection")

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
miracle_add_compile_fail_test(
  MetaParentNoParent compile-fail/meta-parent-no-parent.cxx
  "parent() requires a reflection with a parent")
miracle_add_compile_fail_test(
  MetaQueryMapVoid compile-fail/meta-query-map-void.cxx
  "Query::map() cannot return void")
miracle_add_compile_fail_test(
  MetaQueryMaterializeReference
  compile-fail/meta-query-materialize-reference.cxx
  "Query::materialize() requires a copy-constructible object value")
miracle_add_compile_fail_test(
  MetaQueryTemporaryArray compile-fail/meta-query-temporary-array.cxx
  "Query cannot borrow a temporary array")
miracle_add_compile_fail_test(
  MetaQueryDanglingNestedQuery
  compile-fail/meta-query-dangling-nested-query.cxx
  "read of variable whose lifetime has ended")
miracle_add_compile_fail_test(
  MetaQueryPersistenceMoveOnly
  compile-fail/meta-query-persistence-move-only.cxx
  "Query persistence barrier requires a copy-constructible owning logical value"
)
