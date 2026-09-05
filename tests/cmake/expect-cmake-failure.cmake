cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED SCRIPT)
  message(FATAL_ERROR "expect-cmake-failure.cmake requires -DSCRIPT=...")
endif()
if(NOT DEFINED EXPECTED)
  message(FATAL_ERROR "expect-cmake-failure.cmake requires -DEXPECTED=...")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -P "${SCRIPT}"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE output
  ERROR_VARIABLE error)

if(result EQUAL 0)
  message(FATAL_ERROR "Expected '${SCRIPT}' to fail, but it succeeded")
endif()

string(CONCAT combined "${output}" "${error}")
string(FIND "${combined}" "${EXPECTED}" position)
if(position EQUAL -1)
  message(
    FATAL_ERROR
      "'${SCRIPT}' failed for the wrong reason.\n"
      "Expected diagnostic fragment: ${EXPECTED}\n"
      "Actual output:\n${combined}")
endif()
