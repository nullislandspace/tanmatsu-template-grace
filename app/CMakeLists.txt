cmake_minimum_required(VERSION 3.10)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(app)

# Add -fPIC to ALL component compilations so they can be linked into a shared object
# Use local-exec TLS model since there's no dynamic linker on bare-metal
idf_build_set_property(COMPILE_OPTIONS "-fPIC" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-ftls-model=local-exec" APPEND)
# Remove __thread for PIC compatibility (only pax_gfx.c uses it)
# TLS relocations are incompatible with shared objects on bare-metal RISC-V
idf_build_set_property(COMPILE_OPTIONS "-D__thread=" APPEND)

# After ESP-IDF builds all components, automatically collect all .a files
# and re-link them as a shared object (app.so).
# Dependencies are managed via idf_component.yml — no manual .a file lists needed.
# --gc-sections strips unused code, --start-group handles circular deps.

set(ESP_IDF_LIB_DIR ${CMAKE_BINARY_DIR}/esp-idf)

add_custom_target(app_so ALL
    COMMAND ${CMAKE_COMMAND}
        -DESP_IDF_LIB_DIR=${ESP_IDF_LIB_DIR}
        -DCC=${CMAKE_C_COMPILER}
        -DSIZE=${CMAKE_SIZE}
        -DLD_SCRIPT=${CMAKE_CURRENT_SOURCE_DIR}/app.ld
        -DFAKELIB_DIR=${CMAKE_CURRENT_SOURCE_DIR}/fakelib
        -DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/app.so
        -DMAP_FILE=${CMAKE_CURRENT_BINARY_DIR}/app.map
        -P ${CMAKE_CURRENT_SOURCE_DIR}/link_app_so.cmake
    DEPENDS __idf_main
    COMMENT "Linking app.so"
    VERBATIM
)
