#!/bin/bash
# Generate kbelf symbol tables and fake .so libraries
# Run from main/ directory: bash symbol_export.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR/symbol_export"

assembler="$(find ${IDF_TOOLS_PATH:-../../esp-idf-tools} -name riscv32-esp-elf-cc | head -1)"
if [ -z "$assembler" ]; then
    echo "ERROR: Could not find riscv32-esp-elf-cc assembler"
    echo "Make sure IDF_TOOLS_PATH is set or esp-idf-tools exists"
    exit 1
fi
echo "Assembler: $assembler"

# Ensure output directories exist
mkdir -p ../../app/fakelib

for libname in *; do
    [ -f "$libname" ] || continue
    echo "Exporting symbols in $libname"
    ident=$(echo "$libname" | sed 's/-/_/g')
    ../symbol_export.py \
        --symbols "$libname" \
        --kbelf "../../components/kbelf/src/kbelf_lib_${ident}.c" --kbelf-id "app_elf_lib_${ident}" --kbelf-path "lib${libname}.so" \
        --lib "../../app/fakelib/lib${libname}.so" \
        --assembler "$assembler" \
        -F=-march=rv32imafc_zicsr_zifencei_xesppie -F=-mabi=ilp32f
done

# Generate exported_symbols.cmake to prevent linker from stripping exported symbols
echo "Generating exported_symbols.cmake..."
cmake_file="../exported_symbols.cmake"
echo "# AUTO-GENERATED - do not edit" > "$cmake_file"
echo "# Force-include exported symbols so linker doesn't strip them" >> "$cmake_file"
echo 'target_link_options(${COMPONENT_LIB} INTERFACE' >> "$cmake_file"
for libname in *; do
    [ -f "$libname" ] || continue
    while IFS= read -r line; do
        line=$(echo "$line" | sed 's/#.*//' | xargs)
        [ -z "$line" ] && continue
        # Handle renamed symbols (app_sym = host_sym)
        if echo "$line" | grep -q '='; then
            sym=$(echo "$line" | cut -d= -f1 | xargs)
        else
            sym="$line"
        fi
        echo "    \"-Wl,--undefined=$sym\"" >> "$cmake_file"
    done < "$libname"
done
echo ")" >> "$cmake_file"

echo "Done. Generated files:"
echo "  main/kbelf_lib_*.c      (symbol tables for graceloader)"
echo "  app/fakelib/lib*.so     (fake libraries for app linking)"
echo "  main/exported_symbols.cmake (linker flags)"
