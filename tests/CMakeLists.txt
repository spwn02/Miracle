if(NOT TARGET Switch::Switch)
  find_package(Switch CONFIG QUIET)
endif()

if(NOT TARGET Switch::Switch)
  include(FetchContent)

  FetchContent_Declare(
    Switch
    GIT_REPOSITORY https://github.com/spwn02/Switch.git
    GIT_TAG 2985cc7575f7ba458170684b3df25e4f5493204a
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(Switch)
endif()

if(NOT TARGET Switch::Switch)
  message(
    FATAL_ERROR
      "MIRACLE_BUILD_TESTS requires Switch::Switch"
      "Provide an existing target, an installed Switch package, "
      "FETCHCONTENT_SOURCE_DIR_SWITCH, or the standalone Switch repository.")
endif()

add_executable(MiracleTests main.cxx bitflags.cxx debug.cxx error.cxx fs.cxx
                            reflection.cxx)

target_compile_features(MiracleTests PRIVATE cxx_std_26)

set_target_properties(
  MiracleTests
  PROPERTIES CXX_EXTENSIONS OFF
             CXX_STANDARD_REQUIRED ON
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD ON)

target_link_libraries(MiracleTests PRIVATE Miracle::Miracle Switch::Switch)

# The filesystem tests intentionally write only below the build tree.
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests")

add_test(NAME Miracle.Tests COMMAND MiracleTests)
set_tests_properties(Miracle.Tests PROPERTIES WORKING_DIRECTORY
                                              "${CMAKE_CURRENT_BINARY_DIR}")
