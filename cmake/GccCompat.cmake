# GCC compatibility layer for Miracle.
#
# libstdc++ 16 does not yet provide std::hive, so on this branch a pinned
# upstream plf::hive revision is used as a drop-in backend instead. Kept out
# of the shared CMakeLists.txt build graph (see its trailing include()) so
# that syncing 'master' into this branch never has to merge through it.

if(NOT MIRACLE_CAPABILITY_STD_HIVE)
  include(FetchContent)

  # a110d8b31c2cd3e496d06dcf59417229512e9dc6 was force-pushed away upstream;
  # repin to a currently-reachable revision on mattreecebentley/plf_hive.
  set(miracle_plf_hive_revision "c5c66b417967c493b706ce755472a8314ca56b81")
  FetchContent_Declare(
    plf_hive
    GIT_REPOSITORY https://github.com/mattreecebentley/plf_hive.git
    GIT_TAG "${miracle_plf_hive_revision}"
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(plf_hive)

  if(NOT EXISTS "${plf_hive_SOURCE_DIR}/plf_hive.h")
    message(
      FATAL_ERROR
        "Miracle's std_hive compatibility backend requires plf_hive.h, but "
        "the pinned plf_hive source tree does not contain it.")
  endif()

  target_sources(
    Miracle
    PUBLIC FILE_SET miracle_modules
           TYPE CXX_MODULES
           FILES src/public/compat/HivePlf.ixx)

  target_include_directories(
    Miracle SYSTEM
    PUBLIC "$<BUILD_INTERFACE:${plf_hive_SOURCE_DIR}>"
           "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Miracle/compat>")

  install(FILES "${plf_hive_SOURCE_DIR}/plf_hive.h"
          DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/Miracle/compat")
  install(
    FILES "${plf_hive_SOURCE_DIR}/LICENSE.md"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/Miracle/licenses"
    RENAME plf_hive-LICENSE.md)

  message(
    STATUS "Miracle Hive backend: plf::hive (${miracle_plf_hive_revision})")
else()
  target_sources(
    Miracle
    PUBLIC FILE_SET miracle_modules
           TYPE CXX_MODULES
           FILES src/public/compat/HiveNative.ixx)

  message(STATUS "Miracle Hive backend: std::hive")
endif()
