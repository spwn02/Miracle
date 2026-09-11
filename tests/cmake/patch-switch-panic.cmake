if(NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "patch-switch-panic.cmake requires SOURCE_DIR")
endif()

# Miracle intentionally removes Miracle::fatal. The pinned Switch baseline
# predates that rename, so adapt only the legacy call spelling in the fetched
# test dependency. Switch is not part of the Miracle production target.
file(GLOB_RECURSE switch_sources "${SOURCE_DIR}/src/*.ixx"
     "${SOURCE_DIR}/src/*.cxx")

set(replacement_count 0)
foreach(source IN LISTS switch_sources)
  file(READ "${source}" content)
  string(REGEX MATCHALL "fatal\\(" legacy_calls "${content}")
  list(LENGTH legacy_calls source_replacements)

  if(source_replacements GREATER 0)
    string(REPLACE "fatal(" "panic(" content "${content}")
    file(WRITE "${source}" "${content}")
    math(EXPR replacement_count "${replacement_count} + ${source_replacements}")
  endif()
endforeach()

if(replacement_count EQUAL 0)
  message(
    STATUS
      "Pinned Switch no longer requires the fatal-to-panic compatibility patch")
else()
  message(
    STATUS "Patched ${replacement_count} pinned Switch fatal call(s) to panic")
endif()
