cmake_minimum_required(VERSION 3.25)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/Features.cmake")

miracle_configure_build_features()
if(MIRACLE_BUILD_FEATURE_NAMES
   OR MIRACLE_BUILD_FEATURE_ARGUMENTS
   OR MIRACLE_BUILD_FEATURE_SOURCES
   OR MIRACLE_BUILD_FEATURE_PUBLIC_LIBRARIES
   OR MIRACLE_BUILD_FEATURE_PRIVATE_LIBRARIES)
  message(
    FATAL_ERROR
      "Empty feature registry did not resolve to an empty build universe")
endif()

miracle_compute_feature_build_identity(first)
miracle_compute_feature_build_identity(second)
string(LENGTH "${first}" identity_length)
if(NOT first STREQUAL second
   OR NOT first MATCHES "^[0-9a-f]+$"
   OR NOT identity_length EQUAL 64)
  message(
    FATAL_ERROR "Empty feature build identity is not deterministic SHA-256")
endif()
