#!/bin/bash
# Extract headers from a graceloader build for use by the app template.
# Usage: bash tools/extract_headers.sh /path/to/tanmatsu-graceloader
#
# This copies the necessary header files from the graceloader's
# managed_components and ESP-IDF into this template's include/ directory.
# Run this when graceloader APIs change.

set -e

GRACELOADER_PATH="${1:?Usage: extract_headers.sh <path-to-tanmatsu-graceloader>}"
INCLUDE_DIR="$(cd "$(dirname "$0")/.." && pwd)/include"
BUILD_DIR="$GRACELOADER_PATH/build/tanmatsu"

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "ERROR: compile_commands.json not found in $BUILD_DIR"
    echo "Build the graceloader first: cd $GRACELOADER_PATH && make build"
    exit 1
fi

echo "Extracting headers to $INCLUDE_DIR"
rm -rf "$INCLUDE_DIR"
mkdir -p "$INCLUDE_DIR"

# Extract all -I include paths from compile_commands.json
# This is the authoritative list of directories the compiler used
INCLUDE_DIRS=$(grep -oP '(?<=-I)[^ "]+' "$BUILD_DIR/compile_commands.json" | sort -u)

count=0
for dir in $INCLUDE_DIRS; do
    if [ -d "$dir" ]; then
        # Recursively copy all headers and .inc files preserving directory structure
        (cd "$dir" && find . -type f \( -name "*.h" -o -name "*.inc" -o -name "*.hpp" \) -print0) | \
        while IFS= read -r -d '' file; do
            dest_dir="$INCLUDE_DIR/$(dirname "$file")"
            mkdir -p "$dest_dir"
            cp "$dir/$file" "$dest_dir/"
        done
        count=$((count + 1))
    fi
done

echo "  Processed $count include directories"

# Copy sdkconfig.h from the build
if [ -f "$BUILD_DIR/config/sdkconfig.h" ]; then
    cp "$BUILD_DIR/config/sdkconfig.h" "$INCLUDE_DIR/"
    echo "  Copied sdkconfig.h"
fi

echo ""
total=$(find "$INCLUDE_DIR" -name "*.h" -o -name "*.inc" | wc -l)
echo "Done. Extracted $total header files."
