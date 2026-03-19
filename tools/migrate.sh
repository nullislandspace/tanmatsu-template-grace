#!/bin/bash
# Migration script: Move existing app code to the Graceloader template structure
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "=== Graceloader Migration Script ==="
echo ""

# Check if app/main/main.c already exists with non-template content
if [ -f "$PROJECT_DIR/app/main/main.c" ]; then
    echo "app/main/main.c already exists."
    echo "If you want to re-run migration, delete it first."
    exit 1
fi

# Ensure directories exist
mkdir -p "$PROJECT_DIR/app/main"

# Copy main.c to app
if [ -f "$PROJECT_DIR/main/main.c.original" ]; then
    echo "Found main.c.original, using that as source"
    cp "$PROJECT_DIR/main/main.c.original" "$PROJECT_DIR/app/main/main.c"
else
    echo "Note: main/main.c is now the graceloader. If you have your app code"
    echo "in a git stash or backup, copy it to app/main/main.c manually."
    echo ""
    echo "For new projects, app/main/main.c already contains the template app."
fi

echo ""
echo "=== Migration Complete ==="
echo ""
echo "Your app code should be in: app/main/main.c"
echo "The graceloader is in:      main/main.c (do not modify)"
echo ""
echo "Build with:"
echo "  make build      # Build graceloader"
echo "  make build-app  # Build app library"
echo ""
echo "See PORTING.md for more details."
