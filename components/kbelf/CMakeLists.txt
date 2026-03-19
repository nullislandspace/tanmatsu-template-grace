idf_build_get_property(idf_arch IDF_TARGET_ARCH)
if("${idf_arch}" STREQUAL "riscv")

# Collect all generated kbelf_lib_*.c files
file(GLOB KBELF_LIB_SRCS "src/kbelf_lib_*.c")

idf_component_register(
    SRCS
        "src/kbelf.c"
        "src/kbelf_dyn.c"
        "src/kbelf_file.c"
        "src/kbelf_inst.c"
        "src/kbelf_reloc.c"
        "src/port/riscv.c"
        "src/kbelfx.c"
        "src/fastopen.c"
        ${KBELF_LIB_SRCS}
    INCLUDE_DIRS
        "include"
        "src"
    REQUIRES
        freertos
    PRIV_REQUIRES
        esp_hw_support
        heap
)
endif()
