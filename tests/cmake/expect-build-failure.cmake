cmake_minimum_required(VERSION 3.25)

foreach(required BUILD_DIR TARGET EXPECTED)
  if(NOT DEFINED ${required})
    message(FATAL_ERROR "expect-build-failure.cmake requires -D${required}=...")
  endif()
endforeach()

execute_process(
  COMMAND "${CMAKE_COMMAND}" --build "${BUILD_DIR}" --target "${TARGET}"
  RESULT_VARIABLE build_result
  OUTPUT_VARIABLE build_stdout
  ERROR_VARIABLE build_stderr)

if(build_result EQUAL 0)
  message(
    FATAL_ERROR
      "Compile-fail target '${TARGET}' unexpectedly built successfully")
endif()

set(output "${build_stdout}\n${build_stderr}")
string(FIND "${output}" "${EXPECTED}" expected_position)
if(expected_position EQUAL -1)
  message(
    FATAL_ERROR
      "Compile-fail target '${TARGET}' failed for the wrong reason.\n"
      "Expected diagnostic fragment: ${EXPECTED}\n"
      "Compiler output:\n${output}")
endif()
