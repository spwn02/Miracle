if(NOT TARGET Switch::Switch)
  find_package(Switch CONFIG QUIET)
endif()

if(NOT TARGET Switch::Switch)
  include(FetchContent)

  FetchContent_Declare(
    Switch
    GIT_REPOSITORY https://github.com/spwn02/Switch.git
    GIT_TAG 8941033a9fea0936fb1525ecf42f354ae916575c
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
  main.cpp
  bitflags.cpp
  debug.cpp
  diagnostic.cpp
  error.cpp
  fs.cpp
  memory-profiling.cpp
  range.cpp
  iter.cpp
  meta/semantics/meta.cpp
  meta/query/sources.cpp
  meta/query/barriers.cpp
  meta/query/lifetimes.cpp
  meta/query/interop.cpp
  reflection.cpp
  static-string.cpp
  feature.cpp
  color.cpp
  format.cpp)

target_compile_features(MiracleTests PRIVATE cxx_std_26)

set_target_properties(
  MiracleTests
  PROPERTIES CXX_EXTENSIONS OFF
             CXX_STANDARD_REQUIRED ON
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD ON)

target_link_libraries(MiracleTests PRIVATE Miracle::Miracle Switch::Switch)

# ALG-MOD-001: provider/ownership/acyclicity/forbidden-edge validation over the actual sources.
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
  add_test(NAME Miracle.Modules.Graph
           COMMAND "${Python3_EXECUTABLE}"
                   "${PROJECT_SOURCE_DIR}/scripts/ci/module-graph.py" --root
                   "${PROJECT_SOURCE_DIR}")
endif()

# Every primary module must be consumable through a direct import alone: no umbrella, no transitive
# reliance on another module's re-export. One probe per module; all are built with the normal test build.
file(GLOB miracle_import_probes CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/import/*.cpp")
foreach(probe IN LISTS miracle_import_probes)
  get_filename_component(probe_name "${probe}" NAME_WE)
  add_executable("MiracleImportProbe${probe_name}" "${probe}")
  target_compile_features("MiracleImportProbe${probe_name}" PRIVATE cxx_std_26)
  set_target_properties(
    "MiracleImportProbe${probe_name}"
    PROPERTIES CXX_EXTENSIONS OFF
               CXX_STANDARD_REQUIRED ON
               CXX_SCAN_FOR_MODULES ON
               CXX_MODULE_STD ON)
  target_link_libraries("MiracleImportProbe${probe_name}"
                        PRIVATE Miracle::Miracle)
  add_test(NAME "Miracle.Import.${probe_name}"
           COMMAND "MiracleImportProbe${probe_name}")
endforeach()

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

miracle_add_meta_parity_test(Types types.cpp)
miracle_add_meta_parity_test(Storage storage.cpp)
miracle_add_meta_parity_test(Names names.cpp)
miracle_add_meta_parity_test(Operators operators.cpp)
miracle_add_meta_parity_test(Layout layout.cpp)
miracle_add_meta_parity_test(Access access.cpp)
miracle_add_meta_parity_test(Members members.cpp)
miracle_add_meta_parity_test(Annotations annotations.cpp)
miracle_add_meta_parity_test(Predicates predicates.cpp)
miracle_add_meta_parity_test(TypeQueries type-queries.cpp)
miracle_add_meta_parity_test(TypeTransforms type-transforms.cpp)
miracle_add_meta_parity_test(Values values.cpp)
miracle_add_meta_parity_test(Substitution substitution.cpp)
miracle_add_meta_parity_test(Generation generation.cpp)
miracle_add_meta_parity_test(Reflect reflect.cpp)
miracle_add_meta_parity_test(Surface surface.cpp)
miracle_add_meta_parity_test(Comprehensive std.cpp)

add_subdirectory(meta/hardening)

# The filesystem tests intentionally write only below the build tree.
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests")

add_test(NAME Miracle.Tests COMMAND MiracleTests)
set_tests_properties(Miracle.Tests PROPERTIES WORKING_DIRECTORY
                                              "${CMAKE_CURRENT_BINARY_DIR}")

# Panic is terminal by contract, so each overload runs in its own subprocess.
add_executable(MiraclePanicProbe panic-probe.cpp)
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
add_executable(MiracleContractProbe contract-probe.cpp)
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
  QueryTemporaryArray meta/query/compile-fail/temporary-array.cpp
  "Query cannot borrow a temporary array")
miracle_add_meta_compile_fail_test(
  QueryBorrowedLocalEscape meta/query/compile-fail/borrowed-local-escape.cpp
  "lifetime has ended")
miracle_add_meta_compile_fail_test(
  QueryReferenceMaterialize meta/query/compile-fail/reference-materialize.cpp
  "Query::materialize() requires a copy-constructible object value")
miracle_add_meta_compile_fail_test(
  QueryNonStructuralBarrier meta/query/compile-fail/non-structural-barrier.cpp
  "Query persistence barrier requires a structurally promotable stored element")
miracle_add_meta_compile_fail_test(
  QueryMoveOnlyBarrier meta/query/compile-fail/move-only-barrier.cpp
  "Query persistence barrier requires a copy-constructible owning logical value"
)
miracle_add_meta_compile_fail_test(
  QueryLocalReferenceBarrier
  meta/query/compile-fail/local-reference-barrier.cpp
  "provided value cannot be represented by a reflection")

miracle_add_compile_fail_test(Disabled compile-fail/feature-disabled.cpp
                              "error[FEA007]: feature `tracing` is not enabled")
miracle_add_compile_fail_test(
  Cycle compile-fail/feature-cycle.cpp
  "error[FEA005]: feature dependency cycle: cycle-a --> cycle-b --> cycle-c --> cycle-a")
miracle_add_compile_fail_test(
  Conflict compile-fail/feature-conflict.cpp
  "error[FEA006]: feature conflict: `first` conflicts with `second`")
miracle_add_compile_fail_test(
  DiagnosticNegative compile-fail/diagnostic-negative.cpp
  "because 'NegativeCode' does not satisfy 'DiagnosticCode'")
miracle_add_compile_fail_test(
  DiagnosticAlignWithoutPrefix compile-fail/diagnostic-align-without-prefix.cpp
  "because 'InvalidAlignment' does not satisfy 'DiagnosticCode'")
miracle_add_compile_fail_test(
  DiagnosticNonEnum compile-fail/diagnostic-non-enum.cpp
  "because 'NotAnEnum' does not satisfy 'DiagnosticCode'")
miracle_add_compile_fail_test(
  TypesWithoutNumeric compile-fail/types-without-numeric.cpp
  "must be imported from module 'Miracle.Numeric'")
miracle_add_compile_fail_test(
  DiagnosticEmptyPrefix compile-fail/diagnostic-empty-prefix.cpp
  "because 'EmptyPrefix' does not satisfy 'DiagnosticCode'")
miracle_add_compile_fail_test(
  FormatOrderFieldAfterNote compile-fail/format-order-field-after-note.cpp "no matching member function for call to 'field'")
miracle_add_compile_fail_test(
  FormatOrderContextAfterNote compile-fail/format-order-context-after-note.cpp "no matching member function for call to 'context'")
miracle_add_compile_fail_test(
  FormatOrderSecondContext compile-fail/format-order-second-context.cpp "no matching member function for call to 'context'")
miracle_add_compile_fail_test(
  FormatOrderNoteAfterHelp compile-fail/format-order-note-after-help.cpp "no matching member function for call to 'note'")
miracle_add_compile_fail_test(
  FormatEmptyFieldKey compile-fail/format-empty-field-key.cpp "no matching member function for call to 'field'")
miracle_add_compile_fail_test(
  FormatAppendToDiagnostic compile-fail/format-append-to-diagnostic.cpp "no matching member function for call to 'append'")
miracle_add_compile_fail_test(
  FormatStaticAssertMessage compile-fail/format-static-assert-message.cpp "static assertion failed: error[FMT007]: bad Iter pipeline")
miracle_add_compile_fail_test(
  FormatStyledRuntimeView compile-fail/format-styled-runtime-view.cpp "no matching function for call to 'styled'")
miracle_add_compile_fail_test(
  FormatFragmentRuntimeView compile-fail/format-fragment-runtime-view.cpp
  "no matching function for call to 'fragment'")
miracle_add_compile_fail_test(
  FormatOrderFieldAfterHelp compile-fail/format-order-field-after-help.cpp
  "no matching member function for call to 'field'")
miracle_add_compile_fail_test(
  FormatCapacityOverflow compile-fail/format-capacity-overflow.cpp
  "format::Message capacity must leave room for the terminating NUL without wrapping")
miracle_add_compile_fail_test(
  ColorBraceNarrowing compile-fail/color-brace-narrowing.cpp "cannot be narrowed")
miracle_add_compile_fail_test(
  FormatMessageDanglingStorage compile-fail/format-message-dangling-storage.cpp
  "pointer to subobject of 'message' is not a constant expression")

miracle_add_compile_fail_test(
  StaticStringUnmatchedOpen compile-fail/static-string-unmatched-open.cpp
  "error[SSTR003]: staticFormat contains an unmatched or unsupported brace sequence")
miracle_add_compile_fail_test(
  StaticStringUnmatchedClose compile-fail/static-string-unmatched-close.cpp
  "error[SSTR003]: staticFormat contains an unmatched or unsupported brace sequence")
miracle_add_compile_fail_test(
  StaticStringBadPlaceholder compile-fail/static-string-bad-placeholder.cpp
  "error[SSTR003]: staticFormat contains an unmatched or unsupported brace sequence")
miracle_add_compile_fail_test(
  StaticStringPlaceholderCount compile-fail/static-string-placeholder-count.cpp
  "error[SSTR004]: staticFormat placeholder count does not match argument count")
miracle_add_compile_fail_test(
  StaticStringWideFormat compile-fail/static-string-wide-format.cpp
  "error[SSTR002]: staticFormat currently requires a char format string")
miracle_add_compile_fail_test(
  StaticStringNontrivialChar compile-fail/static-string-nontrivial-char.cpp
  "error[SSTR001]: BasicStaticString requires a trivial standard-layout character type")

miracle_add_compile_fail_test(
  IterAdaptiveInvocation compile-fail/iter-adaptive-invocation.cpp
  "error[ITR001]: Miracle adaptive invocation requires a callable accepting the item directly")
miracle_add_compile_fail_test(
  IterAccumulatorInvocation compile-fail/iter-accumulator-invocation.cpp
  "error[ITR002]: Miracle adaptive invocation requires a callable accepting the accumulator")
miracle_add_compile_fail_test(
  FeatureDescriptorOptions compile-fail/feature-descriptor-options.cpp
  "error[FEA009]: A Feature may contain at most one description option")
miracle_add_compile_fail_test(
  FeatureBuildSetDuplicate compile-fail/feature-build-set-duplicate.cpp
  "error[FEA008]: BuildFeatureSet cannot contain duplicate feature names")

miracle_add_compile_fail_test(
  MetaRequireName compile-fail/meta-require-name.cpp
  "requireName() requires a reflection with an identifier")
miracle_add_compile_fail_test(
  MetaEnumeratorsNonEnum compile-fail/meta-enumerators-non-enum.cpp
  "enumerators() requires an enum reflection")
miracle_add_compile_fail_test(
  MetaParentNoParent compile-fail/meta-parent-no-parent.cpp
  "parent() requires a reflection with a parent")
miracle_add_compile_fail_test(
  MetaQueryMapVoid compile-fail/meta-query-map-void.cpp
  "Query::map() cannot return void")
miracle_add_compile_fail_test(
  MetaQueryMaterializeReference
  compile-fail/meta-query-materialize-reference.cpp
  "Query::materialize() requires a copy-constructible object value")
miracle_add_compile_fail_test(
  MetaQueryTemporaryArray compile-fail/meta-query-temporary-array.cpp
  "Query cannot borrow a temporary array")
miracle_add_compile_fail_test(
  MetaQueryDanglingNestedQuery
  compile-fail/meta-query-dangling-nested-query.cpp
  "read of variable whose lifetime has ended")
miracle_add_compile_fail_test(
  MetaQueryPersistenceMoveOnly
  compile-fail/meta-query-persistence-move-only.cpp
  "Query persistence barrier requires a copy-constructible owning logical value"
)
