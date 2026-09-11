if(NOT DEFINED PROGRAM OR NOT DEFINED EXPECTED)
  message(FATAL_ERROR "expect-process.cmake requires PROGRAM and EXPECTED")
endif()

set(command "${PROGRAM}")
if(DEFINED ARGUMENT AND NOT ARGUMENT STREQUAL "")
  list(APPEND command "${ARGUMENT}")
endif()

execute_process(
  COMMAND ${command}
  RESULT_VARIABLE result
  OUTPUT_VARIABLE output
  ERROR_VARIABLE error)

if(DEFINED EXPECTED_EXIT)
  if(NOT result EQUAL EXPECTED_EXIT)
    message(
      FATAL_ERROR
        "Expected exit ${EXPECTED_EXIT}, got ${result}\nstderr:\n${error}")
  endif()
elseif(result EQUAL 0)
  message(FATAL_ERROR "Expected process failure, but it exited successfully")
endif()

string(FIND "${error}" "${EXPECTED}" found)
if(found EQUAL -1)
  message(
    FATAL_ERROR "Expected stderr to contain '${EXPECTED}'\nstderr:\n${error}")
endif()
