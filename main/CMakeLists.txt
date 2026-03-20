idf_component_register(
    SRCS
        "main.c"
        "sdcard.c"
    PRIV_REQUIRES
        fatfs
        nvs_flash
        kbelf
        appfs
        esp_driver_sdmmc
        sdmmc
        vfs
        esp_system
        heap
    INCLUDE_DIRS
        "."
)

# Force-include exported symbols so the linker doesn't strip them.
# These symbols are only referenced at runtime by the dynamically-loaded app.
include(${CMAKE_CURRENT_SOURCE_DIR}/exported_symbols.cmake OPTIONAL)

# Pass the app slug name so the graceloader knows its fallback path
if(DEFINED APP_SLUG_NAME)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        GRACELOADER_APP_SLUG="${APP_SLUG_NAME}"
    )
endif()

# Force-include libc functions the app.so needs but graceloader doesn't use directly
target_link_options(${COMPONENT_LIB} INTERFACE
    "-Wl,--undefined=asprintf"
    "-Wl,--undefined=exit"
    "-Wl,--undefined=getenv"
    "-Wl,--undefined=vsnprintf"
)

idf_build_set_property(COMPILE_OPTIONS "-Wno-error=unused-variable" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=unused-const-variable" APPEND)
