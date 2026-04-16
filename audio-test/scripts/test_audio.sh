#!/bin/bash
# Test audio on Linux (requires SDL2_mixer)
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REIMPL_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
SONGS_DIR="${1:-$REIMPL_DIR/audio-test/songs}"
SOUNDS_DIR="${2:-$REIMPL_DIR/originals/installed/extracted/Sounds}"

cd "$REIMPL_DIR/reimpl"
gcc -o test_audio_native test_audio.c \
    -I/usr/include/SDL2 \
    -lSDL2_mixer -lSDL2 -lm -D_REENTRANT

SDL_AUDIODRIVER=dummy ./test_audio_native "$SONGS_DIR" "$SOUNDS_DIR"
