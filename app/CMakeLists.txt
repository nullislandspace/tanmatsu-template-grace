cmake_minimum_required(VERSION 3.10)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(app)

# Add -fPIC to ALL component compilations so they can be linked into a shared object
# Remove __thread keyword (TLS relocations are incompatible with -shared on RISC-V)
idf_build_set_property(COMPILE_OPTIONS "-fPIC" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-D__thread=" APPEND)
