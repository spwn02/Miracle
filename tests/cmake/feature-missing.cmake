cmake_minimum_required(VERSION 3.25)
include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/Features.cmake")
miracle_register_build_feature(NAME root DEFAULT DEPENDS absent)
miracle_configure_build_features()
