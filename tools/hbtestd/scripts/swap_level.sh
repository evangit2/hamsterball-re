#!/usr/bin/env bash
# Swap a custom MESHWORLD level file into the game directory.
# Usage: swap_level.sh CUSTOM_LEVEL.MESHWORLD [LEVEL_NAME]
# Example: swap_level.sh Custom-TestPlane.MESHWORLD Level1
#
# If LEVEL_NAME is omitted, defaults to Level1 (Warm-Up Race).
# Creates a .bak backup of the original level if one doesn't exist.

set -euo pipefail

GAME_DIR="${HBTESTD_GAME_DIR:-$HOME/hamsterball-re/originals/installed/extracted}"
LEVELS_DIR="$GAME_DIR/Levels"

CUSTOM="$1"
LEVEL_NAME="${2:-Level1}"
TARGET="$LEVELS_DIR/${LEVEL_NAME}.MESHWORLD"
BACKUP="${TARGET}.bak"

if [ ! -f "$CUSTOM" ]; then
    echo "ERROR: Custom level file not found: $CUSTOM" >&2
    exit 1
fi

if [ ! -d "$LEVELS_DIR" ]; then
    echo "ERROR: Levels directory not found: $LEVELS_DIR" >&2
    exit 1
fi

# Create backup if it doesn't exist
if [ -f "$TARGET" ] && [ ! -f "$BACKUP" ]; then
    echo "Backing up original: $TARGET → $BACKUP"
    cp "$TARGET" "$BACKUP"
fi

# Copy custom level
echo "Installing: $CUSTOM → $TARGET"
cp "$CUSTOM" "$TARGET"
echo "Done! Custom level will load as $LEVEL_NAME in the game."
echo ""
echo "To restore the original later:"
echo "  cp '$BACKUP' '$TARGET'"
