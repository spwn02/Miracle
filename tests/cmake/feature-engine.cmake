cmake_minimum_required(VERSION 3.25)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/Features.cmake")

# Simulate the capability output normally produced by Capabilities.cmake.
set(MIRACLE_CAPABILITY_REFLECTION_CORE TRUE)

miracle_register_build_feature(NAME core DEFAULT DESCRIPTION
                               "Core test feature")
miracle_register_build_feature(
  NAME
  io
  DEPENDS
  core
  REQUIRES_CAPABILITIES
  reflection_core
  SOURCES
  src/public/Io.ixx
  PUBLIC_LIBRARIES
  Example::Public
  PRIVATE_LIBRARIES
  Example::Private)
miracle_register_build_feature(
  NAME
  diagnostics
  IMPLIES
  core
  OPTIONAL_DEPENDS
  metrics
  GROUPS
  all)
miracle_register_build_feature(NAME metrics SOURCES src/public/Metrics.ixx
                               PRIVATE_LIBRARIES Example::Disabled)
miracle_register_feature_group(NAME all MEMBERS io)

set(MIRACLE_FEATURE_IO
    ON
    CACHE BOOL "" FORCE)
set(MIRACLE_FEATURE_GROUP_ALL
    ON
    CACHE BOOL "" FORCE)
miracle_configure_build_features()

set(expected core diagnostics io)
if(NOT MIRACLE_BUILD_FEATURE_NAMES STREQUAL expected)
  message(
    FATAL_ERROR
      "Resolved features '${MIRACLE_BUILD_FEATURE_NAMES}' != '${expected}'")
endif()
if(NOT MIRACLE_BUILD_FEATURE_ARGUMENTS STREQUAL
   "\"core\", \"diagnostics\", \"io\"")
  message(
    FATAL_ERROR
      "Unexpected C++ feature arguments: ${MIRACLE_BUILD_FEATURE_ARGUMENTS}")
endif()
if(NOT MIRACLE_BUILD_FEATURE_SOURCES STREQUAL "src/public/Io.ixx")
  message(
    FATAL_ERROR "Unexpected feature sources: ${MIRACLE_BUILD_FEATURE_SOURCES}")
endif()

if(NOT MIRACLE_BUILD_FEATURE_PUBLIC_LIBRARIES STREQUAL "Example::Public")
  message(
    FATAL_ERROR
      "Unexpected public feature libraries: ${MIRACLE_BUILD_FEATURE_PUBLIC_LIBRARIES}"
  )
endif()
if(NOT MIRACLE_BUILD_FEATURE_PRIVATE_LIBRARIES STREQUAL "Example::Private")
  message(
    FATAL_ERROR
      "Unexpected private feature libraries: ${MIRACLE_BUILD_FEATURE_PRIVATE_LIBRARIES}"
  )
endif()

miracle_compute_feature_build_identity(identity_with_capability reflection_core)
set(MIRACLE_CAPABILITY_REFLECTION_CORE FALSE)
miracle_compute_feature_build_identity(identity_without_capability
                                       reflection_core)
if(identity_with_capability STREQUAL identity_without_capability)
  message(FATAL_ERROR "Feature build identity ignored capability state")
endif()
