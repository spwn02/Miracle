include_guard(GLOBAL)

function(miracle_check_capabilities)
  file(SHA256 "${CMAKE_CURRENT_FUNCTION_LIST_FILE}" capability_contract_hash)

  set(capability_toolchain_hash "")

  if(CMAKE_TOOLCHAIN_FILE AND EXISTS "${CMAKE_TOOLCHAIN_FILE}")
    file(SHA256 "${CMAKE_TOOLCHAIN_FILE}" capability_toolchain_hash)
  endif()

  string(
    CONCAT capability_signature_input
           "${CMAKE_CXX_COMPILER}|"
           "${CMAKE_CXX_COMPILER_VERSION}|"
           "${CMAKE_CXX_FLAGS}|"
           "${CMAKE_BUILD_TYPE}|"
           "${CMAKE_TOOLCHAIN_FILE}|"
           "${capability_toolchain_hash}|"
           "${capability_contract_hash}")

  string(SHA256 capability_signature "${capability_signature_input}")

  unset(capability_signature_input)

  set(capability_root
      "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/MiracleCapabilities/${capability_signature}"
  )
  set(capability_source_dir "${capability_root}/source")
  set(capability_build_dir "${capability_root}/build")

  file(MAKE_DIRECTORY "${capability_source_dir}")

  set(capability_names
      import_std
      reflection_core
      reflection_queries
      reflection_annotations
      reflection_static_storage
      expansion_statements
      std_vocabulary
      std_hive)

  set(capability_import_std_description
      "C++26 import std and standard-library module consumption")
  set(capability_reflection_core_description
      "P2996 reflection and reflection splicing")
  set(capability_reflection_queries_description
      "member/function reflection queries used by Miracle metadata")
  set(capability_reflection_annotations_description
      "reflection annotations and annotation extraction")
  set(capability_reflection_static_storage_description
      "define_static_array and define_static_string")
  set(capability_expansion_statements_description
      "C++26 template for expansion statements")
  set(capability_std_vocabulary_description
      "expected, flat containers, and inplace_vector")
  set(capability_std_hive_description "C++26 std::hive")

  file(
    WRITE "${capability_source_dir}/import_std.cxx"
    [=[
import std;

auto main() -> int {
  const std::vector<int> values{1, 2, 3};
  return values.size() == 3 ? 0 : 1;
}
]=])

  file(
    WRITE "${capability_source_dir}/reflection_core.cxx"
    [=[
import std;

constexpr std::meta::info reflected = ^^int;
static_assert(std::meta::is_type(reflected));

using Reflected = typename[:reflected:];
static_assert(std::same_as<Reflected, int>);

auto main() -> int {
  return 0;
}
]=])

  file(
    WRITE "${capability_source_dir}/reflection_queries.cxx"
    [=[
import std;

struct Sample final {
  int value;
};

auto sampleFunction(double) -> long;

constexpr auto members = std::define_static_array(
    std::meta::nonstatic_data_members_of(
        ^^Sample, std::meta::access_context::unchecked()));
constexpr auto parameters =
    std::define_static_array(std::meta::parameters_of(^^sampleFunction));

static_assert(members.size() == 1);
static_assert(std::meta::identifier_of(members.front()) == "value");
static_assert(parameters.size() == 1);
static_assert(std::meta::is_function_parameter(parameters.front()));

using Return = typename[:std::meta::return_type_of(^^sampleFunction):];
static_assert(std::same_as<Return, long>);

auto main() -> int {
  return 0;
}
]=])

  file(
    WRITE "${capability_source_dir}/reflection_annotations.cxx"
    [=[
import std;

struct Marker final {
  int value;
};

[[= Marker{7}]]
auto annotated() -> void;

constexpr auto annotations =
    std::define_static_array(std::meta::annotations_of(^^annotated));

static_assert(annotations.size() == 1);
static_assert(std::meta::is_annotation(annotations.front()));
static_assert(std::meta::extract<Marker>(annotations.front()).value == 7);

auto main() -> int {
  return 0;
}
]=])

  file(
    WRITE "${capability_source_dir}/reflection_static_storage.cxx"
    [=[
import std;

consteval auto staticText() -> std::string_view {
  constexpr std::string_view value{"miracle"};
  return {std::define_static_string(value), value.size()};
}

consteval auto staticValues() {
  return std::define_static_array(std::array{1, 2, 3});
}

static_assert(staticText() == "miracle");
static_assert(staticValues().size() == 3);
static_assert(staticValues()[2] == 3);

auto main() -> int {
  return 0;
}
]=])

  file(
    WRITE "${capability_source_dir}/expansion_statements.cxx"
    [=[
import std;

constexpr auto values =
    std::define_static_array(std::array{1, 2, 3});

consteval auto enumeratingExpansion() -> int {
    int result{};

    template for (constexpr int value : {1, 2, 3}) {
        result += value;
    }

    return result;
}

consteval auto staticRangeExpansion() -> int {
    int result{};

    template for (constexpr int value : values) {
        result += value;
    }

    return result;
}

static_assert(enumeratingExpansion() == 6);
static_assert(staticRangeExpansion() == 6);

auto main() -> int {
    return 0;
}
]=])

  file(
    WRITE "${capability_source_dir}/std_vocabulary.cxx"
    [=[
#include <expected>
#include <flat_map>
#include <flat_set>
#include <inplace_vector>

auto main() -> int {
  std::expected<int, int> result{1};

  std::flat_map<int, int> map;
  map.emplace(1, 2);

  std::flat_set<int> set;
  set.emplace(3);

  std::inplace_vector<int, 4> values;
  values.push_back(4);

  return result.value() + map.at(1) + *set.begin() + values.front() == 10 ? 0 : 1;
}
]=])

  file(
    WRITE "${capability_source_dir}/std_hive.cxx"
    [=[
#include <hive>

auto main() -> int {
  std::hive<int> values;
  values.insert(42);
  return values.size() == 1 && *values.begin() == 42 ? 0 : 1;
}
]=])

  set(capability_targets)
  foreach(name IN LISTS capability_names)
    string(APPEND capability_targets "miracle_add_capability(${name})\n")
  endforeach()

  file(
    WRITE "${capability_source_dir}/CMakeLists.txt"
    "cmake_minimum_required(VERSION 4.4 FATAL_ERROR)\n"
    "\n"
    "set(CMAKE_CXX_STANDARD 26)\n"
    "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n"
    "set(CMAKE_CXX_EXTENSIONS OFF)\n"
    "set(CMAKE_CXX_SCAN_FOR_MODULES ON)\n"
    "set(CMAKE_CXX_MODULE_STD ON)\n"
    "\n"
    "if(CMAKE_VERSION VERSION_LESS 4.5)\n"
    "  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD\n"
    "      \"f35a9ac6-8463-4d38-8eec-5d6008153e7d\")\n"
    "endif()\n"
    "\n"
    "project(MiracleCapabilityProbe LANGUAGES CXX)\n"
    "\n"
    "if(NOT 26 IN_LIST CMAKE_CXX_COMPILER_IMPORT_STD)\n"
    "  message(FATAL_ERROR \"C++26 import std is unavailable\")\n"
    "endif()\n"
    "\n"
    "function(miracle_add_capability name)\n"
    "  add_executable(capability_\${name} \"\${name}.cxx\")\n"
    "  set_target_properties(\n"
    "    capability_\${name}\n"
    "    PROPERTIES CXX_STANDARD 26\n"
    "               CXX_EXTENSIONS OFF\n"
    "               CXX_STANDARD_REQUIRED ON\n"
    "               CXX_SCAN_FOR_MODULES ON\n"
    "               CXX_MODULE_STD ON)\n"
    "endfunction()\n"
    "\n"
    "${capability_targets}")

  set(configure_command "${CMAKE_COMMAND}" -S "${capability_source_dir}" -B
                        "${capability_build_dir}" -G "${CMAKE_GENERATOR}")

  if(CMAKE_TOOLCHAIN_FILE)
    list(APPEND configure_command
         "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
  else()
    list(APPEND configure_command "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")

    if(CMAKE_CXX_FLAGS)
      list(APPEND configure_command "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
    endif()
  endif()

  if(CMAKE_MAKE_PROGRAM)
    list(APPEND configure_command "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")
  endif()

  if(CMAKE_BUILD_TYPE)
    list(APPEND configure_command "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
  endif()

  execute_process(
    COMMAND ${configure_command}
    RESULT_VARIABLE configure_result
    OUTPUT_VARIABLE configure_stdout
    ERROR_VARIABLE configure_stderr)

  file(WRITE "${capability_root}/configure.log"
       "${configure_stdout}\n${configure_stderr}")

  if(NOT configure_result EQUAL 0)
    message(
      FATAL_ERROR
        "Miracle could not configure its C++26 capability probe project.\n"
        "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}\n"
        "Log: ${capability_root}/configure.log")
  endif()

  set(missing_capabilities)
  set(report "{\n  \"schemaVersion\": 1,\n  \"required\": {\n")
  set(first_report_entry TRUE)

  foreach(name IN LISTS capability_names)
    set(build_command "${CMAKE_COMMAND}" --build "${capability_build_dir}"
                      --target "capability_${name}")

    if(CMAKE_CONFIGURATION_TYPES)
      if(CMAKE_TRY_COMPILE_CONFIGURATION)
        list(APPEND build_command --config "${CMAKE_TRY_COMPILE_CONFIGURATION}")
      else()
        list(APPEND build_command --config Debug)
      endif()
    endif()

    execute_process(
      COMMAND ${build_command}
      RESULT_VARIABLE capability_result
      OUTPUT_VARIABLE capability_stdout
      ERROR_VARIABLE capability_stderr)

    set(capability_log "${capability_root}/${name}.log")
    file(WRITE "${capability_log}" "${capability_stdout}\n${capability_stderr}")

    string(TOUPPER "${name}" capability_cache_name)

    if(capability_result EQUAL 0)
      set(capability_value true)
      set("MIRACLE_CAPABILITY_${capability_cache_name}"
          TRUE
          CACHE INTERNAL "Miracle capability ${name}" FORCE)
      message(STATUS "Miracle capability ${name}: yes")
    else()
      set(capability_value false)
      set("MIRACLE_CAPABILITY_${capability_cache_name}"
          FALSE
          CACHE INTERNAL "Miracle capability ${name}" FORCE)
      list(APPEND missing_capabilities
           "${name}|${capability_${name}_description}|${capability_log}")
      message(STATUS "Miracle capability ${name}: no")
    endif()

    if(first_report_entry)
      set(first_report_entry FALSE)
    else()
      string(APPEND report ",\n")
    endif()
    string(APPEND report "    \"${name}\": ${capability_value}")
  endforeach()

  string(APPEND report "\n  }\n}\n")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/MiracleCapabilities.json" "${report}")

  if(missing_capabilities)
    string(
      CONCAT failure_message
             "Miracle cannot build with the selected toolchain.\n" "\n"
             "Missing capabilities required by the current master revision:\n")

    foreach(item IN LISTS missing_capabilities)
      string(REPLACE "|" ";" fields "${item}")
      list(GET fields 0 name)
      list(GET fields 1 description)
      list(GET fields 2 log)
      string(APPEND failure_message "  - ${name}: ${description}\n"
             "    ${log}\n")
    endforeach()

    string(
      APPEND
      failure_message
      "\n"
      "Miracle master targets the complete standardized C++26-and-earlier "
      "model. These probes are the executable requirements of the current "
      "source revision; they are not an exhaustive C++26 conformance suite.\n"
      "Capability report: "
      "${CMAKE_CURRENT_BINARY_DIR}/MiracleCapabilities.json")

    message(FATAL_ERROR "${failure_message}")
  endif()
endfunction()
