# Compile-time Query fixtures compare a direct raw <meta> pipeline with the
# fused Miracle.Meta Query representation. Each variant is isolated in its own
# translation unit so the benchmark driver can measure compiler front-end work.
set(miracle_meta_query_sizes 32 128 512 1024)
set(miracle_meta_query_generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${miracle_meta_query_generated_dir}")

function(miracle_add_meta_query_benchmark variant size)
  set(FIELD_DECLARATIONS "")
  math(EXPR last_member "${size} - 1")
  foreach(index RANGE 0 ${last_member})
    string(APPEND FIELD_DECLARATIONS "  int field${index}{};\n")
  endforeach()

  set(MEMBER_COUNT "${size}")
  math(EXPR REPETITIONS "2048 / ${size}")
  if(REPETITIONS LESS 2)
    set(REPETITIONS 2)
  endif()

  if(variant STREQUAL "RawOnce")
    set(BENCHMARK_BODY
        [=[
static_assert(rawPipeline() == memberCount);
]=])
  elseif(variant STREQUAL "MiracleOnce")
    set(BENCHMARK_BODY
        [=[
static_assert(miraclePipeline() == memberCount);
]=])
  elseif(variant STREQUAL "RawRepeat")
    set(BENCHMARK_BODY
        [=[
template <std::size_t... Index>
consteval auto runRaw(std::index_sequence<Index...>) -> std::size_t {
  return ((rawPipeline() + (Index * 0)) + ... + std::size_t{});
}

static_assert(runRaw(std::make_index_sequence<repetitions>{}) == memberCount * repetitions);
]=])
  elseif(variant STREQUAL "MiracleRepeat")
    set(BENCHMARK_BODY
        [=[
template <std::size_t... Index>
consteval auto runMiracle(std::index_sequence<Index...>) -> std::size_t {
  return ((miraclePipeline() + (Index * 0)) + ... + std::size_t{});
}

static_assert(runMiracle(std::make_index_sequence<repetitions>{}) == memberCount * repetitions);
]=])
  else()
    message(FATAL_ERROR "Unknown Meta Query benchmark variant: ${variant}")
  endif()

  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/source.cxx.in"
    "${miracle_meta_query_generated_dir}/${variant}-${size}.cxx" @ONLY)

  add_executable("MiracleMetaQuery${variant}${size}" EXCLUDE_FROM_ALL
                 "${miracle_meta_query_generated_dir}/${variant}-${size}.cxx")
  target_compile_features("MiracleMetaQuery${variant}${size}"
                          PRIVATE cxx_std_26)
  set_target_properties(
    "MiracleMetaQuery${variant}${size}"
    PROPERTIES CXX_EXTENSIONS OFF
               CXX_STANDARD_REQUIRED ON
               CXX_SCAN_FOR_MODULES ON
               CXX_MODULE_STD ON)
  target_link_libraries("MiracleMetaQuery${variant}${size}"
                        PRIVATE Miracle::Miracle)
endfunction()

foreach(size IN LISTS miracle_meta_query_sizes)
  foreach(variant IN ITEMS RawOnce MiracleOnce RawRepeat MiracleRepeat)
    miracle_add_meta_query_benchmark("${variant}" "${size}")
  endforeach()
endforeach()

# Exception-versus-predicate control-flow fixtures use the same mixed reflection
# sequence so both strategies answer the identical semantic question.
set(miracle_meta_query_control_dir "${CMAKE_CURRENT_BINARY_DIR}/control")
file(MAKE_DIRECTORY "${miracle_meta_query_control_dir}")
set(META_QUERY_CONTROL_SUBJECTS "")
set(META_QUERY_CONTROL_EXPECTED 0)
math(EXPR meta_query_control_last 127)
foreach(index RANGE 0 ${meta_query_control_last})
  math(EXPR modulo "${index} % 4")
  if(modulo EQUAL 0)
    string(APPEND META_QUERY_CONTROL_SUBJECTS "  ^^int,\n")
  else()
    string(APPEND META_QUERY_CONTROL_SUBJECTS "  ^^Subject,\n")
    math(EXPR META_QUERY_CONTROL_EXPECTED "${META_QUERY_CONTROL_EXPECTED} + 1")
  endif()
endforeach()

foreach(variant IN ITEMS Predicate Exception)
  set(SUBJECTS "${META_QUERY_CONTROL_SUBJECTS}")
  set(EXPECTED "${META_QUERY_CONTROL_EXPECTED}")
  if(variant STREQUAL "Predicate")
    set(CONTROL_FUNCTION
        [=[
consteval auto selectedPath() -> std::size_t {
  std::size_t result{};
  for (const Miracle::meta::Info info : subjects) {
    if (Miracle::meta::name(info).has_value()) {
      ++result;
    }
  }
  return result;
}
]=])
  else()
    set(CONTROL_FUNCTION
        [=[
consteval auto selectedPath() -> std::size_t {
  std::size_t result{};
  for (const Miracle::meta::Info info : subjects) {
    try {
      (void)Miracle::meta::requireName(info);
      ++result;
    } catch (const std::meta::exception &) {
    }
  }
  return result;
}
]=])
  endif()

  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/control-flow.cxx.in"
                 "${miracle_meta_query_control_dir}/${variant}.cxx" @ONLY)

  add_executable("MiracleMetaQueryControl${variant}" EXCLUDE_FROM_ALL
                 "${miracle_meta_query_control_dir}/${variant}.cxx")
  target_compile_features("MiracleMetaQueryControl${variant}"
                          PRIVATE cxx_std_26)
  set_target_properties(
    "MiracleMetaQueryControl${variant}"
    PROPERTIES CXX_EXTENSIONS OFF
               CXX_STANDARD_REQUIRED ON
               CXX_SCAN_FOR_MODULES ON
               CXX_MODULE_STD ON)
  target_link_libraries("MiracleMetaQueryControl${variant}"
                        PRIVATE Miracle::Miracle)
endforeach()
