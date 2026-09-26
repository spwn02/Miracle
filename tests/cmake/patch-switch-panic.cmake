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

# Miracle's replaces pre-Meta variable-template vocabulary with the public
# Miracle.Meta module. The pinned Switch test baseline still consumes those
# historical names. Adapt Switch locally instead of re-exporting legacy aliases
# from Miracle's production module.
set(annotations_source "${SOURCE_DIR}/src/public/Annotations.ixx")
file(READ "${annotations_source}" annotations_content)
string(FIND "${annotations_content}"
            "Miracle Phase-5 test compatibility facade" compat_position)
if(compat_position EQUAL -1)
  file(READ "${CMAKE_CURRENT_LIST_DIR}/switch-meta-compat.inc" meta_compat)
  string(
    REPLACE
      "using namespace Miracle;\n\n"
      "using namespace Miracle;\n\n// Miracle Phase-5 test compatibility facade.\n${meta_compat}\n\n"
      annotations_content
      "${annotations_content}")
  file(WRITE "${annotations_source}" "${annotations_content}")
  message(
    STATUS "Patched pinned Switch for the Phase-5 Miracle.Meta vocabulary")
endif()

# Redirect the pinned dependency's old Miracle::meta helper spellings to the
# Switch-local compatibility namespace. The std::meta repair handles matches
# inside qualified standard reflection names after the textual substitutions.
file(GLOB_RECURSE switch_sources "${SOURCE_DIR}/src/*.ixx"
     "${SOURCE_DIR}/src/*.cxx")
foreach(source IN LISTS switch_sources)
  file(READ "${source}" content)
  string(REPLACE "meta::StaticString" "compat::StaticString" content
                 "${content}")
  string(REPLACE "meta::TypeObject" "compat::TypeObject" content "${content}")
  string(REPLACE "meta::Type<" "compat::Type<" content "${content}")
  string(REPLACE "meta::ReturnObject" "compat::ReturnObject" content
                 "${content}")
  string(REPLACE "meta::Return<" "compat::Return<" content "${content}")
  string(REPLACE "meta::identifier" "compat::identifier" content "${content}")
  string(REPLACE "meta::annotations" "compat::annotations" content "${content}")
  string(REPLACE "meta::enumerators" "compat::enumerators" content "${content}")
  string(REPLACE "meta::nsMembers" "compat::nsMembers" content "${content}")
  string(REPLACE "meta::members" "compat::members" content "${content}")
  string(REPLACE "meta::parameters" "compat::parameters" content "${content}")
  string(REPLACE "meta::AccessContext" "compat::AccessContext" content
                 "${content}")
  string(REPLACE "meta::always_false_v" "compat::always_false_v" content
                 "${content}")
  string(REPLACE "std::compat::" "std::meta::" content "${content}")
  file(WRITE "${source}" "${content}")
endforeach()
