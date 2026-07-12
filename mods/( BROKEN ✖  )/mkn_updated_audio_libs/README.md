# Updated Audio Libraries

Replaces Hamsterball's original BASS 2.0 audio library (circa 2003) with a proxy DLL that forwards all calls to the modern **BASS 2.4.18.3** (December 2025).

## What This Does

The game ships with BASS 2.0 — a 20+ year old audio library. This mod provides a proxy `bass.dll` that transparently translates the old API calls to the new BASS 2.4 API, giving the game:

- **WASAPI output** (default on Windows Vista+) instead of legacy DirectSound
- 20+ years of bug fixes and audio quality improvements
- Better device detection and handling
- Modern audio driver compatibility
- Better Wine/Android compatibility (relevant for mobile players)
- MO3, OGG, and WAV format support (all still fully supported in BASS 2.4)

## API Translation

The game uses 3 functions that were removed/renamed in BASS 2.3+. The proxy translates them:

| Game calls (BASS 2.0) | Proxy forwards to (BASS 2.4) | Translation |
|---|---|---|
| `BASS_ChannelSetAttributes(handle, attrib, vol100, pan100)` | `BASS_ChannelSetAttribute(handle, VOL, vol/100.0)` + `BASS_ChannelSetAttribute(handle, PAN, pan/100.0)` | int 0-100 → float 0.0-1.0 |
| `BASS_MusicPlayEx(handle, pos, reset, flags)` | `BASS_ChannelPlay(handle, TRUE)` | Simplified to play+restart |
| `BASS_MusicLoad(mem, file, offset32, len, flags, freq)` | `BASS_MusicLoad(filetype, file, offset64, len, flags, freq)` | DWORD offset → QWORD offset |

The other 7 functions (`BASS_Init`, `BASS_Free`, `BASS_Start`, `BASS_Stop`, `BASS_SetConfig`, `BASS_ErrorGetCode`, `BASS_ChannelStop`) are forwarded directly — their signatures are unchanged between BASS 2.0 and 2.4.

## Installation

1. **Back up** your original `bass.dll` (rename it to `bass_orig.dll`)
2. Copy `bass24.dll` to your game folder and **rename it to `bass_real.dll`**
3. Copy the modded `bass.dll` to your game folder
4. Launch the game

```
Game Folder/
├── Hamsterball.exe
├── bass.dll          ← mod proxy (from this mod)
├── bass_real.dll     ← BASS 2.4.18.3 (renamed from bass24.dll)
├── bass_orig.dll     ← your original BASS 2.0 (backup)
└── ...
```

## Files

| File | Description |
|---|---|
| `bass.dll` | Proxy DLL (90 KB) — translates BASS 2.0 → 2.4 API calls |
| `bass24.dll` | Official BASS 2.4.18.3 library (145 KB) — rename to `bass_real.dll` |
| `updated_audio_libs.c` | Full source code |

## Technical Details

### Sound Effects

The game's sound effects (`.ogg` files in `Sounds/`) do **not** use BASS at all — they use DirectSound directly via `SoundDevice_ctor` and `Sound_LoadOgg`. This mod only upgrades the music playback system (MO3 module files). Sound effects will continue to work as before.

### BASS Version Comparison

| | BASS 2.0 (original) | BASS 2.4.18.3 (this mod) |
|---|---|---|
| Year | ~2003 | December 2025 |
| File size | 95 KB | 145 KB |
| Exports | 101 functions | 110 functions |
| Audio backend | DirectSound | WASAPI (Vista+) / DirectSound fallback |
| Mixing | Hardware/software | Software (always) |
| Copyright | 1999-2004 | 1999-2025 |

### Fallback Behavior

If `bass_real.dll` is missing, the proxy returns safe defaults (music init fails gracefully — game shows "music not available" dialog but doesn't crash). Sound effects are unaffected.

## Crash Test Results

- **Wine/Xvfb**: 35s, no crash, no Wine errors ✅
- **Game log**: Only ALSA "no sound card" errors (normal in VM) ✅
- No unimplemented function errors ✅
