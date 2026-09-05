include_guard(GLOBAL)

# Registers one build-selectable Miracle feature.
#
# This is the build-system half of Miracle's Feature engine. It decides which
# heavy module sources/dependencies belong to the configured binary. C++ code
# consumes only the generated typed BuildFeatureSet; these CMake variables never
# become the public feature API.
function(miracle_register_build_feature)
  set(options DEFAULT)
  set(one_value NAME DESCRIPTION)
  set(multi_value
      DEPENDS
      OPTIONAL_DEPENDS
      CONFLICTS
      IMPLIES
      REQUIRES_CAPABILITIES
      GROUPS
      SOURCES
      PUBLIC_LIBRARIES
      PRIVATE_LIBRARIES)
  cmake_parse_arguments(ARG "${options}" "${one_value}" "${multi_value}"
                        ${ARGN})

  if(ARG_UNPARSED_ARGUMENTS)
    message(
      FATAL_ERROR
        "Unknown miracle_register_build_feature arguments: ${ARG_UNPARSED_ARGUMENTS}"
    )
  endif()

  if(NOT ARG_NAME)
    message(FATAL_ERROR "miracle_register_build_feature requires NAME")
  endif()

  if(NOT ARG_NAME MATCHES "^[A-Za-z][A-Za-z0-9_.-]*$")
    message(
      FATAL_ERROR
        "Invalid Miracle feature name '${ARG_NAME}'. Feature names must "
        "start with a letter and contain only letters, digits, '.', '_' or '-'."
    )
  endif()

  get_property(registered GLOBAL PROPERTY MIRACLE_REGISTERED_BUILD_FEATURES)
  if(ARG_NAME IN_LIST registered)
    message(
      FATAL_ERROR "Miracle build feature '${ARG_NAME}' is already registered")
  endif()

  list(APPEND registered "${ARG_NAME}")
  set_property(GLOBAL PROPERTY MIRACLE_REGISTERED_BUILD_FEATURES
                               "${registered}")

  string(MAKE_C_IDENTIFIER "${ARG_NAME}" feature_id)
  string(TOUPPER "${feature_id}" feature_id)

  # CMake cache/property names use a sanitized identifier. Reject two stable
  # names that collapse to the same identifier rather than silently sharing an
  # option (for example `foo-bar` and `foo_bar`).
  get_property(existing_id_name GLOBAL
               PROPERTY "MIRACLE_FEATURE_ID_${feature_id}")
  if(existing_id_name AND NOT existing_id_name STREQUAL ARG_NAME)
    message(
      FATAL_ERROR
        "Miracle feature names '${existing_id_name}' and '${ARG_NAME}' "
        "map to the same build identifier '${feature_id}'")
  endif()
  set_property(GLOBAL PROPERTY "MIRACLE_FEATURE_ID_${feature_id}" "${ARG_NAME}")

  if(ARG_DEFAULT)
    set(default_value ON)
  else()
    set(default_value OFF)
  endif()

  if(ARG_DESCRIPTION)
    set(help "${ARG_DESCRIPTION}")
  else()
    set(help "Enable Miracle build feature '${ARG_NAME}'")
  endif()

  option("MIRACLE_FEATURE_${feature_id}" "${help}" ${default_value})

  set_property(GLOBAL PROPERTY "MIRACLE_FEATURE_${feature_id}_NAME"
                               "${ARG_NAME}")
  set_property(GLOBAL PROPERTY "MIRACLE_FEATURE_${feature_id}_DEPENDS"
                               "${ARG_DEPENDS}")
  set_property(GLOBAL PROPERTY "MIRACLE_FEATURE_${feature_id}_OPTIONAL_DEPENDS"
                               "${ARG_OPTIONAL_DEPENDS}")
  set_property(GLOBAL PROPERTY "MIRACLE_FEATURE_${feature_id}_CONFLICTS"
                               "${ARG_CONFLICTS}")
  set_property(GLOBAL PROPERTY "MIRACLE_FEATURE_${feature_id}_IMPLIES"
                               "${ARG_IMPLIES}")
  set_property(GLOBAL PROPERTY "MIRACLE_FEATURE_${feature_id}_CAPABILITIES"
                               "${ARG_REQUIRES_CAPABILITIES}")
  set_property(GLOBAL PROPERTY "MIRACLE_FEATURE_${feature_id}_GROUPS"
                               "${ARG_GROUPS}")
  set_property(GLOBAL PROPERTY "MIRACLE_FEATURE_${feature_id}_SOURCES"
                               "${ARG_SOURCES}")
  set_property(GLOBAL PROPERTY "MIRACLE_FEATURE_${feature_id}_PUBLIC_LIBRARIES"
                               "${ARG_PUBLIC_LIBRARIES}")
  set_property(GLOBAL PROPERTY "MIRACLE_FEATURE_${feature_id}_PRIVATE_LIBRARIES"
                               "${ARG_PRIVATE_LIBRARIES}")
endfunction()

# Registers a build-level feature group. Enabling a group enables every member;
# the same concept is represented by feature::Group on the C++ side.
function(miracle_register_feature_group)
  set(options DEFAULT)
  set(one_value NAME DESCRIPTION)
  set(multi_value MEMBERS)
  cmake_parse_arguments(ARG "${options}" "${one_value}" "${multi_value}"
                        ${ARGN})

  if(ARG_UNPARSED_ARGUMENTS)
    message(
      FATAL_ERROR
        "Unknown miracle_register_feature_group arguments: ${ARG_UNPARSED_ARGUMENTS}"
    )
  endif()
  if(NOT ARG_NAME)
    message(FATAL_ERROR "miracle_register_feature_group requires NAME")
  endif()
  get_property(groups GLOBAL PROPERTY MIRACLE_REGISTERED_FEATURE_GROUPS)
  if(ARG_NAME IN_LIST groups)
    message(
      FATAL_ERROR "Miracle feature group '${ARG_NAME}' is already registered")
  endif()
  list(APPEND groups "${ARG_NAME}")
  set_property(GLOBAL PROPERTY MIRACLE_REGISTERED_FEATURE_GROUPS "${groups}")

  if(NOT ARG_NAME MATCHES "^[A-Za-z][A-Za-z0-9_.-]*$")
    message(
      FATAL_ERROR
        "Invalid Miracle feature-group name '${ARG_NAME}'. Group names must "
        "start with a letter and contain only letters, digits, '.', '_' or '-'."
    )
  endif()

  string(MAKE_C_IDENTIFIER "${ARG_NAME}" group_id)
  string(TOUPPER "${group_id}" group_id)
  get_property(existing_id_name GLOBAL
               PROPERTY "MIRACLE_FEATURE_GROUP_ID_${group_id}")
  if(existing_id_name AND NOT existing_id_name STREQUAL ARG_NAME)
    message(
      FATAL_ERROR
        "Miracle feature-group names '${existing_id_name}' and '${ARG_NAME}' "
        "map to the same build identifier '${group_id}'")
  endif()
  set_property(GLOBAL PROPERTY "MIRACLE_FEATURE_GROUP_ID_${group_id}"
                               "${ARG_NAME}")

  if(ARG_DEFAULT)
    set(default_value ON)
  else()
    set(default_value OFF)
  endif()
  if(ARG_DESCRIPTION)
    set(help "${ARG_DESCRIPTION}")
  else()
    set(help "Enable Miracle feature group '${ARG_NAME}'")
  endif()
  option("MIRACLE_FEATURE_GROUP_${group_id}" "${help}" ${default_value})
  set_property(GLOBAL PROPERTY "MIRACLE_FEATURE_GROUP_${group_id}_MEMBERS"
                               "${ARG_MEMBERS}")
endfunction()

# Converts a stable feature name to the CMake identifier used by registration
# metadata/cache variables.
function(miracle_feature_id name output)
  string(MAKE_C_IDENTIFIER "${name}" feature_id)
  string(TOUPPER "${feature_id}" feature_id)
  set("${output}"
      "${feature_id}"
      PARENT_SCOPE)
endfunction()

# Resolves one selected build feature recursively. `stack` is propagated by
# value so a dependency cycle can report its complete path without mutable
# global DFS state.
function(miracle_resolve_build_feature name stack)
  get_property(registered GLOBAL PROPERTY MIRACLE_REGISTERED_BUILD_FEATURES)
  if(NOT name IN_LIST registered)
    message(FATAL_ERROR "Miracle feature '${name}' is not registered")
  endif()

  get_property(resolved GLOBAL PROPERTY MIRACLE_RESOLVED_BUILD_FEATURES)
  if(name IN_LIST resolved)
    return()
  endif()

  if(name IN_LIST stack)
    list(FIND stack "${name}" cycle_start)
    list(SUBLIST stack ${cycle_start} -1 cycle)
    list(APPEND cycle "${name}")
    list(JOIN cycle " --> " cycle_text)
    message(FATAL_ERROR "Miracle feature dependency cycle: ${cycle_text}")
  endif()

  set(next_stack ${stack})
  list(APPEND next_stack "${name}")

  miracle_feature_id("${name}" feature_id)
  get_property(dependencies GLOBAL
               PROPERTY "MIRACLE_FEATURE_${feature_id}_DEPENDS")
  get_property(implied GLOBAL PROPERTY "MIRACLE_FEATURE_${feature_id}_IMPLIES")

  foreach(dependency IN LISTS dependencies implied)
    miracle_resolve_build_feature("${dependency}" "${next_stack}")
  endforeach()

  get_property(capabilities GLOBAL
               PROPERTY "MIRACLE_FEATURE_${feature_id}_CAPABILITIES")
  foreach(capability IN LISTS capabilities)
    string(TOUPPER "${capability}" capability_id)
    if(NOT DEFINED MIRACLE_CAPABILITY_${capability_id}
       OR NOT MIRACLE_CAPABILITY_${capability_id})
      message(
        FATAL_ERROR
          "Miracle feature '${name}' requires unavailable capability '${capability}'"
      )
    endif()
  endforeach()

  get_property(resolved GLOBAL PROPERTY MIRACLE_RESOLVED_BUILD_FEATURES)
  list(APPEND resolved "${name}")
  list(REMOVE_DUPLICATES resolved)
  set_property(GLOBAL PROPERTY MIRACLE_RESOLVED_BUILD_FEATURES "${resolved}")
endfunction()

# Resolves registered feature options/groups into the canonical C++ build-set
# argument list plus the heavy module sources that must be compiled.
function(miracle_configure_build_features)
  set_property(GLOBAL PROPERTY MIRACLE_RESOLVED_BUILD_FEATURES "")

  get_property(registered GLOBAL PROPERTY MIRACLE_REGISTERED_BUILD_FEATURES)
  get_property(groups GLOBAL PROPERTY MIRACLE_REGISTERED_FEATURE_GROUPS)

  set(requested)
  foreach(name IN LISTS registered)
    miracle_feature_id("${name}" feature_id)
    if(MIRACLE_FEATURE_${feature_id})
      list(APPEND requested "${name}")
    endif()
  endforeach()

  foreach(group IN LISTS groups)
    miracle_feature_id("${group}" group_id)
    if(MIRACLE_FEATURE_GROUP_${group_id})
      get_property(members GLOBAL
                   PROPERTY "MIRACLE_FEATURE_GROUP_${group_id}_MEMBERS")

      # Feature-side GROUPS metadata and explicit group MEMBERS are equivalent
      # selectors. Resolve membership here instead of registration time so
      # features and groups can be declared in either order.
      foreach(feature_name IN LISTS registered)
        miracle_feature_id("${feature_name}" feature_id)
        get_property(feature_groups GLOBAL
                     PROPERTY "MIRACLE_FEATURE_${feature_id}_GROUPS")
        if(group IN_LIST feature_groups)
          list(APPEND members "${feature_name}")
        endif()
      endforeach()

      list(APPEND requested ${members})
    endif()
  endforeach()

  list(REMOVE_DUPLICATES requested)
  foreach(name IN LISTS requested)
    miracle_resolve_build_feature("${name}" "")
  endforeach()

  get_property(enabled GLOBAL PROPERTY MIRACLE_RESOLVED_BUILD_FEATURES)
  list(SORT enabled)

  # Conflicts are evaluated over the complete transitive closure so request
  # order cannot affect configuration validity.
  foreach(name IN LISTS enabled)
    miracle_feature_id("${name}" feature_id)
    get_property(conflicts GLOBAL
                 PROPERTY "MIRACLE_FEATURE_${feature_id}_CONFLICTS")
    foreach(conflict IN LISTS conflicts)
      if(conflict IN_LIST enabled)
        message(
          FATAL_ERROR "Miracle feature '${name}' conflicts with '${conflict}'")
      endif()
    endforeach()
  endforeach()

  set(arguments)
  set(sources)
  set(public_libraries)
  set(private_libraries)
  foreach(name IN LISTS enabled)
    string(REPLACE "\\" "\\\\" escaped "${name}")
    string(REPLACE "\"" "\\\"" escaped "${escaped}")
    list(APPEND arguments "\"${escaped}\"")

    miracle_feature_id("${name}" feature_id)
    get_property(feature_sources GLOBAL
                 PROPERTY "MIRACLE_FEATURE_${feature_id}_SOURCES")
    get_property(feature_public_libraries GLOBAL
                 PROPERTY "MIRACLE_FEATURE_${feature_id}_PUBLIC_LIBRARIES")
    get_property(feature_private_libraries GLOBAL
                 PROPERTY "MIRACLE_FEATURE_${feature_id}_PRIVATE_LIBRARIES")
    list(APPEND sources ${feature_sources})
    list(APPEND public_libraries ${feature_public_libraries})
    list(APPEND private_libraries ${feature_private_libraries})
  endforeach()

  list(JOIN arguments ", " arguments_text)
  list(REMOVE_DUPLICATES sources)
  list(REMOVE_DUPLICATES public_libraries)
  list(REMOVE_DUPLICATES private_libraries)

  set(MIRACLE_BUILD_FEATURE_ARGUMENTS
      "${arguments_text}"
      PARENT_SCOPE)
  set(MIRACLE_BUILD_FEATURE_NAMES
      "${enabled}"
      PARENT_SCOPE)
  set(MIRACLE_BUILD_FEATURE_SOURCES
      "${sources}"
      PARENT_SCOPE)
  set(MIRACLE_BUILD_FEATURE_PUBLIC_LIBRARIES
      "${public_libraries}"
      PARENT_SCOPE)
  set(MIRACLE_BUILD_FEATURE_PRIVATE_LIBRARIES
      "${private_libraries}"
      PARENT_SCOPE)
endfunction()

# Computes the deterministic configuration identity consumed by the generated
# FeatureConfiguration module. The feature list is already canonicalized by
# miracle_configure_build_features; capability values are appended in the
# caller-provided stable vocabulary order.
function(miracle_compute_feature_build_identity output)
  set(identity_input "features=${MIRACLE_BUILD_FEATURE_NAMES}")
  foreach(capability IN LISTS ARGN)
    string(TOUPPER "${capability}" capability_id)
    if(MIRACLE_CAPABILITY_${capability_id})
      set(capability_value true)
    else()
      set(capability_value false)
    endif()
    string(APPEND identity_input "|${capability}=${capability_value}")
  endforeach()
  string(SHA256 identity "${identity_input}")
  set("${output}"
      "${identity}"
      PARENT_SCOPE)
endfunction()
