#!/bin/bash
# Link app.so using ESP-IDF's own library list from build.ninja
# This replicates ESP-IDF's link command but with -shared instead of firmware link scripts
set -e

BUILD_DIR="${1:?Usage: link_app_so.sh <build_dir>}"
APP_DIR="${2:?Usage: link_app_so.sh <build_dir> <app_dir>}"
OUTPUT="${BUILD_DIR}/app.so"

APP_LD="${APP_DIR}/app.ld"
FAKELIB_DIR="${APP_DIR}/fakelib"
VERSION_SCRIPT="${APP_DIR}/app_version.script"

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
    # Pass through -Wl,--whole-archive and -Wl,--no-whole-archive
    case "$item" in
        -Wl,--whole-archive|-Wl,--no-whole-archive) LIBS="$LIBS $item"; continue ;;
    esac
    # Pass through -u (undefined symbol) flags and their arguments
    case "$item" in
        -u) LIBS="$LIBS $item"; continue ;;
    esac
    # Pass through -Wl flags (wraps, undefined, etc) but NOT -Wl,-T linker scripts
    case "$item" in
        -Wl,-T,*) continue ;;
        -Wl,*) LIBS="$LIBS $item"; continue ;;
    esac
    # Skip -l flags for system libs (not PIC, use fakelibs instead)
    case "$item" in
        -lc|-lm|-lgcc|-lstdc++) continue ;;
        -l*) continue ;;
    esac
    # Skip .ld files and non-.a files
    case "$item" in
        *.ld) continue ;;
        *.a) ;;
        *) continue ;;
    esac
    # Skip only libraries with non-PIC assembly or boot-only code
    case "$(basename "$item")" in
        libriscv.a) continue ;;  # Interrupt vectors - hand-written assembly
        libesp_app_format.a|libesp_bootloader_format.a|libapp_update.a) continue ;;  # Boot-only
        libesp_gdbstub.a|libespcoredump.a) continue ;;  # Debug-only
        libnvs_sec_provider.a|librt.a|libcxx.a|libbsp_stub.a) continue ;;  # Unused
        libunity.a|libcmock.a) continue ;;  # Test frameworks
    esac
    # Resolve relative path
    if [[ "$item" != /* ]]; then
        item="${BUILD_DIR}/${item}"
    fi
    [ -f "$item" ] && LIBS="$LIBS $item"
done

# Find ESP-IDF linker scripts that provide ROM symbols and peripheral addresses
IDF_PATH="${IDF_PATH:-$(pwd)/esp-idf}"
ROM_LD_DIR="${IDF_PATH}/components/esp_rom/esp32p4/ld"
SOC_LD_DIR="${IDF_PATH}/components/soc/esp32p4/ld"

# Collect all ROM linker scripts
ROM_LD_FLAGS=""
for ld in "${ROM_LD_DIR}"/esp32p4.rom*.ld; do
    [ -f "$ld" ] && ROM_LD_FLAGS="$ROM_LD_FLAGS -Wl,-T,$ld"
done

echo "Linking app.so..."
riscv32-esp-elf-gcc \
    -shared -nostdlib \
    -Wl,--gc-sections \
    -Wl,-Bsymbolic \
    -Wl,--version-script="${VERSION_SCRIPT}" \
    -Wl,-T,"${SOC_LD_DIR}/esp32p4.peripherals.ld" \
    $ROM_LD_FLAGS \
    -Wl,-T,"${APP_LD}" \
    -Wl,-Map="${BUILD_DIR}/app_so.map" \
    -L"${FAKELIB_DIR}" \
    $LIBS \
    -lall \
    -o "${OUTPUT}"

echo "app.so linked successfully:"
riscv32-esp-elf-size "${OUTPUT}"
ls -lh "${OUTPUT}"
