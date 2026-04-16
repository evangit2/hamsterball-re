---
description: Reimplement Hamsterball audio from BASS/DirectSound to SDL2_mixer — pitfalls, volume fade patterns, MinGW build issues, and Windows testing workflow.
name: hamsterball-audio-reimplementation
category: gaming
trigger: audio reimplementation, BASS replacement, SDL2_mixer, hamsterball audio system
---

# Hamsterball Audio Reimplementation: BASS → SDL2_mixer

## Original Audio Architecture
- **BASS** (un4seen.com) — DirectSound-based audio library
- Music: MO3 format via `BASS_MusicLoad`
- SFX: OGG samples via `BASS_SampleLoad`  
- Volume: per-channel BASS_ChannelSlideAttribute for fade-in/out
- Initialization: `BASS_Init(-1, 44100, ...)` (default device, 44100Hz)

## SDL2_mixer Replacement

### Key API Mappings
| BASS | SDL2_mixer |
|---|---|
| BASS_Init(-1, 44100) | Mix_OpenAudio(44100, AUDIO_S16, 2, 2048) |
| BASS_MusicLoad(file, MO3) | Mix_LoadMUS(wav); pre-render MO3→WAV offline |
| BASS_SampleLoad(file, OGG) | Mix_LoadWAV(ogg); Mix_Chunk |
| BASS_ChannelPlay(handle, flags) | Mix_PlayMusic(Mix_Music*, loops) |
| BASS_ChannelPause(handle) | Mix_PauseMusic() |
| BASS_ChannelStop(handle) | Mix_HaltMusic() |
| BASS_ChannelSetVolume / Slide | Mix_VolumeMusic(vol); dual-buffer lerp for fade |
| BASS_SampleFree(handle) | Mix_FreeChunk(chunk) |
| BASS_MusicFree(handle) | Mix_FreeMusic(music) |

### Volume Fade-In Pattern (Critical)
BASS `BASS_ChannelSlideAttribute` smoothly interpolates volume over time.
SDL2_mixer has no equivalent — `Mix_FadeInMusic` only controls fade duration, not per-frame volume.

**Solution: dual-buffer volume lerp**
```c
typedef struct {
    float current;    // what we're actually playing at
    float target;     // what we want (0.0 to 1.0)
    float acc;        // accumulator for linear interp
    bool fading_in;
} MusicFadeCtx;

void audio_music_fade_in(AudioContext* ctx, float target, float duration_ms) {
    ctx->fade_ctx.target = target;
    ctx->fade_ctx.fading_in = true;
    // In game loop or audio thread: linearly lerp current → target
    // Use accumulator += (delta_time / duration_ms) * target
    // Then Mix_VolumeMusic((int)(current * 128))
}
```

### Music/SFX Channel Separation
SDL2_mixer reserves channel 0 for music. SFX use channels 1–255.
```c
// Music on channel 0 (automatic in SDL2_mixer)
Mix_PlayMusic(music, loops);
Mix_VolumeMusic((int)(vol * 128));

// SFX on any other channel  
Mix_PlayChannel(1, sfx_chunk, 0);
Mix_Volume(1, (int)(vol * 128));
```

## Windows/MinGW Build Issues

### Issue: mingw-libs SDL2 headers broken
**Symptom:** Cross-compiling with `x86_64-w64-mingw32-gcc` fails with redefinitions in SDL2 headers (e.g., `DECLSPEC` redefinition, conflicting `SDL_aligned_alloc` declarations).

**Cause:** The `mingw-libs` package on Linux ships corrupted/broken SDL2 headers.

**Workaround:**
```bash
# Copy native Linux SDL2 headers over the broken MinGW ones
cp -r /usr/include/SDL2 /usr/x86_64-w64-mingw32/include/SDL2
cp /usr/lib/x86_64-linux-gnu/libSDL2.a /usr/x86_64-w64-mingw32/lib/
cp /usr/lib/x86_64-linux-gnu/libSDL2_mixer.a /usr/x86_64-w64-mingw32/lib/
# For cross-compile, link: -lSDL2 -lSDL2_mixer -lwinmm -lgdi32 -lcomdlg32 -lole32 -luuid -ldxguid
```

### Issue: WinMain@16 undefined when linking SDL2_mixer
**Symptom:** Undefined reference to `WinMain@16` after adding `-lSDL2_mixer`.

**Cause:** SDL2 defines its own `main()` macro that conflicts with MinGW's `WinMain`. When linking SDL2_mixer (which depends on SDL2main), the linker can't find the WinMain entry point.

**Fix:** Ensure `#define SDL_main` is set before including SDL headers, OR add `-lmingw32` explicitly in the link order:
```bash
x86_64-w64-mingw32-gcc ... -lSDL2main -lSDL2 -lSDL2_mixer -lmingw32 -lwinmm -lcomdlg32 -lole32 -luuid -ldxguid
```
Also ensure `int main(int argc, char* argv[])` exists in the codebase (not `WinMain`).

## MO3 → WAV Pre-rendering Pipeline
Original MO3 music files must be pre-rendered to WAV for SDL2_mixer compatibility.
The game used a custom MO3 player to extract subsongs.

Songs are stored in the `data/music/` directory alongside the original MO3 files.
Naming convention: `00_main_theme.wav`, `02_hamster_nation.wav`, etc.

## Testing
```bash
# Linux native test
gcc -o test_audio test_audio.c audio.c $(sdl2-config --cflags --libs) -lSDL2_mixer -lm
./test_audio

# Windows cross-compile test
x86_64-w64-mingw32-gcc -o test_audio.exe test_audio.c audio.c \
    -I/usr/x86_64-w64-mingw32/include/SDL2 \
    -L/usr/x86_64-w64-mingw32/lib \
    -lSDL2main -lSDL2 -lSDL2_mixer -lmingw32 -lwinmm -lcomdlg32 -lole32 -luuid -ldxguid \
    -static-libgcc -static-libstdc++
```
