#!/bin/bash
# Link app.so using ESP-IDF's own library list from build.ninja
# This replicates ESP-IDF's link command but with -shared instead of firmware link scripts
set -e

BUILD_DIR="${1:?Usage: link_app_so.sh <build_dir>}"
APP_LD="${2:?Usage: link_app_so.sh <build_dir> <app.ld>}"
FAKELIB_DIR="${3:?Usage: link_app_so.sh <build_dir> <app.ld> <fakelib_dir>}"
OUTPUT="${BUILD_DIR}/app.so"

# Extract the LINK_LIBRARIES from build.ninja for app.elf
# This is the exact library list ESP-IDF determined
LINK_LIBS=$(grep "LINK_LIBRARIES = " "${BUILD_DIR}/build.ninja" | head -1 | sed 's/.*LINK_LIBRARIES = //')

# Convert to array, resolve relative paths, filter for .a files
LIBS=""
for item in $LINK_LIBS; do
    # Skip firmware-specific linker flags
    case "$item" in
        -T|-nostartfiles|-fno-*|--*) continue ;;
    esac
    # Pass through -Wl,--whole-archive, -Wl,--no-whole-archive, -u flags, -l flags
    case "$item" in
        -Wl,--whole-archive|-Wl,--no-whole-archive) LIBS="$LIBS $item"; continue ;;
        -u) LIBS="$LIBS $item"; continue ;;
        -lc|-lm|-lgcc|-lstdc++) LIBS="$LIBS $item"; continue ;;
        -l*) LIBS="$LIBS $item"; continue ;;
        -Wl,*) LIBS="$LIBS $item"; continue ;;
    esac
    # Skip .ld files and non-.a files
    case "$item" in
        *.ld) continue ;;
        *.a) ;;
        *) continue ;;
    esac
    # Skip system libs with non-PIC assembly
    case "$(basename "$item")" in
        libriscv.a|libesp_system.a|libesp_rom.a|libbootloader_support.a) continue ;;
        libesp_app_format.a|libesp_bootloader_format.a|libapp_update.a) continue ;;
        libesp_gdbstub.a|libespcoredump.a|libnvs_sec_provider.a) continue ;;
        libesp_security.a|librt.a|libcxx.a|libbsp_stub.a) continue ;;
        libunity.a|libcmock.a) continue ;;
    esac
    # Resolve relative path
    if [[ "$item" != /* ]]; then
        item="${BUILD_DIR}/${item}"
    fi
    [ -f "$item" ] && LIBS="$LIBS $item"
done

echo "Linking app.so..."
riscv32-esp-elf-gcc \
    -shared -nostdlib \
    -Wl,--gc-sections \
    -Wl,-Bsymbolic \
    -Wl,-T,"${APP_LD}" \
    -Wl,-Map="${BUILD_DIR}/app_so.map" \
    $LIBS \
    -o "${OUTPUT}"

echo "app.so linked successfully:"
riscv32-esp-elf-size "${OUTPUT}"
ls -lh "${OUTPUT}"
