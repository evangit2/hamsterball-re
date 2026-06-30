# rsksJIT — Universal Ref Loader (Logging Build)

Based on RodentRacer's v3 JIT mesh injection mod (`mods/universal-ref-loader/`).
This is a separate project that adds comprehensive logging without modifying the original.

## What It Does

A bass.dll proxy that hooks `Scene_CreateDynamicObjects` at `0x0040C4BA` and logs
**everything** the universal ref loader does:

- Every ref name seen by the factory dispatch
- Every board slot check (offset, value before/after JIT injection)
- Every JIT mesh load (path, cache hit/miss, success/fail)
- Every clone operation (Level_CloneTree)
- Every difficulty bypass (save/restore App+0x23C)
- Every safety check pass/fail
- Board pointer, vtable pointer, App pointer values

## Log Output

Log file: `Z:\tmp\ref_loader_log.txt` (maps to `C:\tmp\ref_loader_log.txt` on Windows,
or `/tmp/ref_loader_log.txt` on Wine/Linux).

## Usage

1. Copy `bass.dll` into the Hamsterball game directory (backup original first!)
2. Ensure `bass_real.dll` is the **true original** BASS library (89710 bytes), NOT
   another proxy DLL
3. Launch the game
4. Navigate to a race level
5. Read `C:\tmp\ref_loader_log.txt`

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll jit_log_mod.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## BASS Proxy Notes

The original game imports 20 BASS functions. The v3 original proxy only forwarded 15.
This build adds 5 missing exports that the game's import table requires:

- `BASS_Start` — Start audio output
- `BASS_Stop` — Stop audio output
- `BASS_MusicPlayEx` — Play music module (extended)
- `BASS_ErrorGetCode` — Get last error code
- `BASS_ChannelSetAttributes` — Note: plural form (game imports this, not singular `BASS_ChannelSetAttribute`)

Without all 20 exports, Wine aborts with "unimplemented function BASS.dll.BASS_Start".

## Testing on Wine/llvmpipe

The game renders **black** on Wine with llvmpipe software rendering. Key navigation
via xdotool does not reliably work when the screen is black. This mod should be tested
on **real Windows** or with a GPU-accelerated Wine setup.

The `hbtestd` MCP tool can crash-test the DLL (verifies it loads without crashing),
but cannot navigate to a race level on llvmpipe due to the black screen issue.

## Original Project

This is based on `mods/universal-ref-loader/` by RodentRacer. The original project
should NOT be modified — this is a separate fork for logging/analysis purposes.
