# Miracle compiler-cost fixtures. Each target is a separate translation unit so
# the benchmark driver can time the compiler process directly after all imported
# module BMIs are already available.
set(miracle_meta_cache_sizes 32 128 512 1024)
set(miracle_meta_cache_generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${miracle_meta_cache_generated_dir}")

function(miracle_add_meta_cache_benchmark variant size)
  set(FIELD_DECLARATIONS "")
  math(EXPR last_member "${size} - 1")
  foreach(index RANGE 0 ${last_member})
    string(APPEND FIELD_DECLARATIONS "  int field${index}{};\n")
  endforeach()

  set(MEMBER_COUNT "${size}")

  # Keep the uncached repeated workload roughly constant (~2048 reflected
  # members) so the larger subjects remain below compiler constexpr limits while
  # every repeated case still observes cache reuse.
  math(EXPR REPETITIONS "2048 / ${size}")
  if(REPETITIONS LESS 2)
    set(REPETITIONS 2)
  endif()

  # Every fixture imports Miracle.Meta, including the raw-standard baselines.
  # That charges all variants equally for BMI loading and isolates the cost of
  # the reflection expression itself rather than module-import setup.
  set(MIRACLE_IMPORT "import Miracle.Meta;")

  if(variant STREQUAL "RawOnce")
    set(BENCHMARK_BODY
        [=[
constexpr auto reflected = std::define_static_array(
    std::meta::nonstatic_data_members_of(^^Subject, std::meta::access_context::unchecked()));
static_assert(reflected.size() == memberCount);
]=])
  elseif(variant STREQUAL "MiracleOnce")
    set(BENCHMARK_BODY
        [=[
constexpr auto reflected = Miracle::meta::fields<Subject>(Miracle::Access::unchecked());
static_assert(reflected.size() == memberCount);
]=])
  elseif(variant STREQUAL "MiracleRawOnce")
    set(BENCHMARK_BODY
        [=[
constexpr auto reflected = Miracle::meta::fields(^^Subject, Miracle::Access::unchecked());
static_assert(reflected.size() == memberCount);
]=])
  elseif(variant STREQUAL "RawRepeat")
    set(BENCHMARK_BODY
        [=[
template <std::size_t Index>
consteval auto reflectRepeatedly() -> std::size_t {
  constexpr auto reflected = std::define_static_array(
      std::meta::nonstatic_data_members_of(^^Subject, std::meta::access_context::unchecked()));
  return reflected.size() + (Index * 0);
}

template <std::size_t... Index>
consteval auto run(std::index_sequence<Index...>) -> std::size_t {
  return (reflectRepeatedly<Index>() + ... + std::size_t{});
}

static_assert(run(std::make_index_sequence<repetitions>{}) == memberCount * repetitions);
]=])
  elseif(variant STREQUAL "MiracleRepeat")
    set(BENCHMARK_BODY
        [=[
template <std::size_t Index>
consteval auto reflectRepeatedly() -> std::size_t {
  constexpr auto reflected = Miracle::meta::fields<Subject>(Miracle::Access::unchecked());
  return reflected.size() + (Index * 0);
}

template <std::size_t... Index>
consteval auto run(std::index_sequence<Index...>) -> std::size_t {
  return (reflectRepeatedly<Index>() + ... + std::size_t{});
}

static_assert(run(std::make_index_sequence<repetitions>{}) == memberCount * repetitions);
]=])
  elseif(variant STREQUAL "MiracleRawRepeat")
    set(BENCHMARK_BODY
        [=[
template <std::size_t Index>
consteval auto reflectRepeatedly() -> std::size_t {
  constexpr auto reflected = Miracle::meta::fields(^^Subject, Miracle::Access::unchecked());
  return reflected.size() + (Index * 0);
}

template <std::size_t... Index>
consteval auto run(std::index_sequence<Index...>) -> std::size_t {
  return (reflectRepeatedly<Index>() + ... + std::size_t{});
}

static_assert(run(std::make_index_sequence<repetitions>{}) == memberCount * repetitions);
]=])
  elseif(variant STREQUAL "RawVisibleRepeat")
    set(BENCHMARK_BODY
        [=[
template <std::size_t Index>
consteval auto reflectRepeatedly() -> std::size_t {
  constexpr auto reflected = std::define_static_array(
      std::meta::nonstatic_data_members_of(^^Subject, std::meta::access_context::unprivileged()));
  return reflected.size() + (Index * 0);
}

template <std::size_t... Index>
consteval auto run(std::index_sequence<Index...>) -> std::size_t {
  return (reflectRepeatedly<Index>() + ... + std::size_t{});
}

static_assert(run(std::make_index_sequence<repetitions>{}) == memberCount * repetitions);
]=])
  elseif(variant STREQUAL "MiracleVisibleRepeat")
    set(BENCHMARK_BODY
        [=[
template <std::size_t Index>
consteval auto reflectRepeatedly() -> std::size_t {
  constexpr auto reflected = Miracle::meta::fields<Subject>(Miracle::Access::unprivileged());
  return reflected.size() + (Index * 0);
}

template <std::size_t... Index>
consteval auto run(std::index_sequence<Index...>) -> std::size_t {
  return (reflectRepeatedly<Index>() + ... + std::size_t{});
}

static_assert(run(std::make_index_sequence<repetitions>{}) == memberCount * repetitions);
]=])
  elseif(variant STREQUAL "MiracleRawVisibleRepeat")
    set(BENCHMARK_BODY
        [=[
template <std::size_t Index>
consteval auto reflectRepeatedly() -> std::size_t {
  constexpr auto reflected = Miracle::meta::fields(^^Subject, Miracle::Access::unprivileged());
  return reflected.size() + (Index * 0);
}

template <std::size_t... Index>
consteval auto run(std::index_sequence<Index...>) -> std::size_t {
  return (reflectRepeatedly<Index>() + ... + std::size_t{});
}

static_assert(run(std::make_index_sequence<repetitions>{}) == memberCount * repetitions);
]=])
  else()
    message(FATAL_ERROR "Unknown Meta cache benchmark variant: ${variant}")
  endif()

  set(source "${miracle_meta_cache_generated_dir}/${variant}-${size}.cxx")
  configure_file(source.cxx.in "${source}" @ONLY NEWLINE_STYLE LF)

  set(target "MiracleMetaCache${variant}${size}")
  add_executable("${target}" EXCLUDE_FROM_ALL "${source}")
  target_compile_features("${target}" PRIVATE cxx_std_26)
  set_target_properties(
    "${target}"
    PROPERTIES CXX_EXTENSIONS OFF
               CXX_STANDARD_REQUIRED ON
               CXX_SCAN_FOR_MODULES ON
               CXX_MODULE_STD ON)
  target_link_libraries("${target}" PRIVATE Miracle::Miracle)
endfunction()

foreach(size IN LISTS miracle_meta_cache_sizes)
  foreach(
    variant IN
    ITEMS RawOnce
          MiracleOnce
          MiracleRawOnce
          RawRepeat
          MiracleRepeat
          MiracleRawRepeat
          RawVisibleRepeat
          MiracleVisibleRepeat
          MiracleRawVisibleRepeat)
    miracle_add_meta_cache_benchmark("${variant}" "${size}")
  endforeach()
endforeach()
