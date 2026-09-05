cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED OUTPUT_DIR)
  message(FATAL_ERROR "feature-configuration.cmake requires -DOUTPUT_DIR=...")
endif()

file(MAKE_DIRECTORY "${OUTPUT_DIR}")
set(MIRACLE_BUILD_FEATURE_ARGUMENTS "\"core\", \"io\"")
set(MIRACLE_FEATURE_BUILD_IDENTITY
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef")
configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/../../src/public/FeatureConfiguration.ixx.in"
  "${OUTPUT_DIR}/FeatureConfiguration.ixx" @ONLY NEWLINE_STYLE LF)

file(READ "${OUTPUT_DIR}/FeatureConfiguration.ixx" generated)
foreach(expected
        "buildSet<\"core\", \"io\">"
        "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef")
  string(FIND "${generated}" "${expected}" position)
  if(position EQUAL -1)
    message(
      FATAL_ERROR "Generated feature configuration is missing '${expected}'")
  endif()
endforeach()
