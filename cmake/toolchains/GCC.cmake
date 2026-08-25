# GCC compatibility toolchain for Miracle's gcc branch.
#
# C++26 mode must be active during compiler identification because GCC's
# reflection implementation rejects -freflection in older language modes. Do not
# add -fmodules here: CMake owns module scanning and injects the appropriate GCC
# module flags per module-aware target.
set(CMAKE_CXX_COMPILER
    "g++"
    CACHE FILEPATH "GCC C++ compiler")
set(CMAKE_CXX_FLAGS_INIT "-std=c++26 -freflection")

# A superproject must enable CMake 4.4's import-std gate before its project()
# call. Keeping it in the toolchain makes the same file usable for standalone
# Miracle and for consumers which add Miracle as a subproject.
if(CMAKE_VERSION VERSION_LESS 4.5 AND NOT DEFINED
                                      CMAKE_EXPERIMENTAL_CXX_IMPORT_STD)
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
endif()
