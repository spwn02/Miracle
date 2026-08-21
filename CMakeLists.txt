cmake_minimum_required(VERSION 4.4 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CMake 4.4 still gates import std support behind its experimental UUID.
# Standalone Miracle can enable it itself because CXX has not been discovered
# yet. A superproject must enable the gate before its own project() call.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR
   AND CMAKE_VERSION LESS 4.5
   AND NOT DEFINED CMAKE_EXPERIMENTAL_CXX_IMPORT_STD)
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
endif()

project(
  Miracle
  VERSION 0.1.0
  DESCRIPTION "A modern C++26 foundation library."
  LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(NOT CMAKE_GENERATOR MATCHES "^Ninja")
  message(
    FATAL_ERROR
      "Miracle 0.1.0 requires a Ninja generator because CMake 4.4 only "
      "supports 'import std;' for Ninja and Ninja Multi-Config.")
endif()

if(NOT 26 IN_LIST CMAKE_CXX_COMPILER_IMPORT_STD)
  message(
    FATAL_ERROR
      "Miracle requires toolchain support for C++26 'import std;'. "
      "When Miracle is a subproject, enable CMake's import-std support before "
      "the top-level project() call.")
endif()

set(miracle_public_compile_options)
set(miracle_private_compile_options
    -Wall
    -Wextra
    -Wpedantic
    -Wunused-local-typedefs
    -Wunused-result
    -Wwrite-strings
    -Wno-missing-designated-field-initializers)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16)
    message(
      FATAL_ERROR "Miracle requires GCC 16 or newer when building with GCC.")
  endif()

  list(APPEND miracle_public_compile_options -fmodules -freflection)

  list(
    APPEND
    miracle_private_compile_options
    -Wconversion-null
    -Woverlength-strings
    -Wpointer-arith
    -Wvarargs
    -Wvla)

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  list(APPEND miracle_public_compile_options -freflection-latest)

  list(
    APPEND
    miracle_private_compile_options
    -Woverlength-strings
    -Wpointer-arith
    -Wvarargs
    -Wvla
    -Wmodule-file-config-mismatch)

else()
  message(
    FATAL_ERROR
      "Miracle 0.1.0 currently supports GCC 16+ and reflection-enabled Clang. "
      "Selected compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}."
  )
endif()

option(MIRACLE_WARNINGS_AS_ERRORS
       "Treat warnings emitted while compiling Miracle itself as errors" OFF)

if(MIRACLE_WARNINGS_AS_ERRORS)
  list(APPEND miracle_private_compile_options -Werror)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(MIRACLE_BUILD_PLATFORM Linux)
  set(miracle_memory_source src/private/linux/Memory.cxx)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(MIRACLE_BUILD_PLATFORM Windows)
  set(miracle_memory_source src/private/windows/Memory.cxx)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(MIRACLE_BUILD_PLATFORM MacOS)
  set(miracle_memory_source src/private/unsupported/Memory.cxx)
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  set(MIRACLE_BUILD_PLATFORM IOS)
  set(miracle_memory_source src/private/unsupported/Memory.cxx)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(MIRACLE_BUILD_PLATFORM Android)
  set(miracle_memory_source src/private/unsupported/Memory.cxx)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  set(MIRACLE_BUILD_PLATFORM FreeBSD)
  set(miracle_memory_source src/private/unsupported/Memory.cxx)
else()
  set(MIRACLE_BUILD_PLATFORM Unsupported)
  set(miracle_memory_source src/private/unsupported/Memory.cxx)
endif()

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" miracle_processor)
if(miracle_processor MATCHES "^(x86_64|amd64)$")
  set(MIRACLE_BUILD_ARCHITECTURE x86_64)
elseif(miracle_processor MATCHES "^(aarch64|arm64)$")
  set(MIRACLE_BUILD_ARCHITECTURE AArch64)
else()
  set(MIRACLE_BUILD_ARCHITECTURE Unknown)
endif()

set(MIRACLE_OPTIMIZED
    "AUTO"
    CACHE STRING "Miracle optimization policy: AUTO, ON, or OFF")
set_property(CACHE MIRACLE_OPTIMIZED PROPERTY STRINGS AUTO ON OFF)

string(TOUPPER "${MIRACLE_OPTIMIZED}" miracle_optimized)
if(NOT miracle_optimized MATCHES "^(AUTO|ON|OFF)$")
  message(FATAL_ERROR "MIRACLE_OPTIMIZED must be AUTO, ON, or OFF; received "
                      "'${MIRACLE_OPTIMIZED}'")
endif()

if(miracle_optimized STREQUAL "ON")
  set(MIRACLE_BUILD_OPTIMIZED true)
elseif(miracle_optimized STREQUAL "OFF")
  set(MIRACLE_BUILD_OPTIMIZED false)
elseif(CMAKE_CONFIGURATION_TYPES)
  message(
    FATAL_ERROR
      "MIRACLE_OPTIMIZED=AUTO is ambiguous with a multi-config generator. "
      "Set MIRACLE_OPTIMIZED explicitly ON or OFF.")
elseif(CMAKE_BUILD_TYPE MATCHES "^(Release|RelWithDebInfo|MinSizeRel)$")
  set(MIRACLE_BUILD_OPTIMIZED true)
else()
  set(MIRACLE_BUILD_OPTIMIZED false)
endif()

set(miracle_generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${miracle_generated_dir}")
configure_file(src/public/Build.ixx.in "${miracle_generated_dir}/Build.ixx"
               @ONLY NEWLINE_STYLE LF)

add_library(Miracle STATIC)
add_library(Miracle::Miracle ALIAS Miracle)

target_compile_features(Miracle PUBLIC cxx_std_26)

set_target_properties(
  Miracle
  PROPERTIES CXX_EXTENSIONS OFF
             CXX_STANDARD_REQUIRED ON
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD ON)

target_sources(
  Miracle
  PUBLIC FILE_SET
         miracle_modules
         TYPE
         CXX_MODULES
         BASE_DIRS
         "${CMAKE_CURRENT_SOURCE_DIR}/src/public"
         "${miracle_generated_dir}"
         FILES
         src/public/Miracle.ixx
         src/public/Types.ixx
         src/public/Error.ixx
         src/public/Fs.ixx
         src/public/Memory.ixx
         src/public/Profiling.ixx
         src/public/Concepts.ixx
         src/public/meta/Meta.ixx
         src/public/meta/Bitflags.ixx
         src/public/meta/Debug.ixx
         "${miracle_generated_dir}/Build.ixx"
  PRIVATE src/private/Error.cxx src/private/Fs.cxx src/private/Profiling.cxx
          "${miracle_memory_source}")

target_compile_options(Miracle PUBLIC ${miracle_public_compile_options})

target_compile_options(
  Miracle
  PUBLIC ${miracle_public_compile_options}
  PRIVATE ${miracle_private_compile_options})

set(miracle_cmake_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/Miracle")

configure_package_config_file(
  cmake/MiracleConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/MiracleConfig.cmake"
  INSTALL_DESTINATION "${miracle_cmake_install_dir}")

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/MiracleConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SemanticVersion)

install(
  TARGETS Miracle
  EXPORT MiracleTargets
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
          FILE_SET miracle_modules
          DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Miracle")

install(
  EXPORT MiracleTargets
  FILE MiracleTargets.cmake
  NAMESPACE Miracle::
  DESTINATION "${miracle_cmake_install_dir}"
  CXX_MODULES_DIRECTORY modules)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/MiracleConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/MiracleConfigVersion.cmake"
        DESTINATION "${miracle_cmake_install_dir}")

install(FILES LICENSE DESTINATION "${CMAKE_INSTALL_DATADIR}/Miracle")

option(MIRACLE_BUILD_EXAMPLES "Build Miracle examples" OFF)
option(MIRACLE_BUILD_TESTS "Build Miracle's self-test suite" OFF)

if(MIRACLE_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(MIRACLE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
