cmake_minimum_required(VERSION 4.4 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_MODULE_STD ON)

if(CMAKE_VERSION VERSION_LESS 4.5 AND NOT DEFINED
                                      CMAKE_EXPERIMENTAL_CXX_IMPORT_STD)
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
endif()

project(MiracleConsumer LANGUAGES CXX)

if(NOT MIRACLE_SOURCE_DIR)
  message(FATAL_ERROR "MIRACLE_SOURCE_DIR must point at a Miracle source tree")
endif()

add_subdirectory("${MIRACLE_SOURCE_DIR}" miracle)

add_executable(miracle_consumer "${CMAKE_CURRENT_LIST_DIR}/../main.cxx")
target_compile_features(miracle_consumer PRIVATE cxx_std_26)
target_link_libraries(miracle_consumer PRIVATE Miracle::Miracle)

set_target_properties(
  miracle_consumer
  PROPERTIES CXX_STANDARD 26
             CXX_STANDARD_REQUIRED ON
             CXX_EXTENSIONS OFF
             CXX_SCAN_FOR_MODULES ON
             CXX_MODULE_STD ON)
