# Cross-compilation toolchain for Graceloader app plugins
# Requires the RISC-V toolchain from ESP-IDF tools (riscv32-esp-elf-*)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv32)

# Find the compiler - search IDF_TOOLS_PATH, local esp-idf-tools, then PATH
# Use CMAKE_CURRENT_LIST_DIR (always the dir of this file) not CMAKE_CURRENT_SOURCE_DIR
# (which changes during try_compile)
file(GLOB _toolchain_hints
    "$ENV{IDF_TOOLS_PATH}/tools/riscv32-esp-elf/*/riscv32-esp-elf/bin"
    "${CMAKE_CURRENT_LIST_DIR}/esp-idf-tools/tools/riscv32-esp-elf/*/riscv32-esp-elf/bin"
)

find_program(RISCV_CC riscv32-esp-elf-gcc
    HINTS ${_toolchain_hints}
)

if(NOT RISCV_CC)
    message(FATAL_ERROR
        "Could not find riscv32-esp-elf-gcc.\n"
        "Make sure ESP-IDF tools are installed and either:\n"
        "  - Set IDF_TOOLS_PATH environment variable, or\n"
        "  - Have esp-idf-tools/ in this directory, or\n"
        "  - source esp-idf/export.sh before running cmake"
    )
endif()

get_filename_component(TOOLCHAIN_DIR ${RISCV_CC} DIRECTORY)

set(CMAKE_C_COMPILER "${TOOLCHAIN_DIR}/riscv32-esp-elf-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_DIR}/riscv32-esp-elf-g++")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_DIR}/riscv32-esp-elf-gcc")
set(CMAKE_AR "${TOOLCHAIN_DIR}/riscv32-esp-elf-ar")
set(CMAKE_RANLIB "${TOOLCHAIN_DIR}/riscv32-esp-elf-ranlib")
set(CMAKE_OBJCOPY "${TOOLCHAIN_DIR}/riscv32-esp-elf-objcopy")
set(CMAKE_SIZE "${TOOLCHAIN_DIR}/riscv32-esp-elf-size")

set(CMAKE_C_FLAGS_INIT "-march=rv32imafc_zicsr_zifencei_xesppie -mabi=ilp32f")
set(CMAKE_CXX_FLAGS_INIT "-march=rv32imafc_zicsr_zifencei_xesppie -mabi=ilp32f")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
