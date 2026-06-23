# Universal Ref Loader — Hamsterball DLL Mod

## What It Does

This mod patches the game's level-object dispatch system to allow **any ref type to be loaded into any level**. Normally, each level only creates objects for its own specific ref types (e.g. SpeedCylinders only in Up Race, Gears only in Impossible Race). This mod replaces the per-level factory dispatch with a universal dispatcher that tries all 13 level factories in sequence until one handles the ref.

## How It Works

1. The game's `Scene_CreateDynamicObjects` function (0x0040C4BA) iterates over all ref points in the loaded MESHWORLD level data
2. For each ref, it calls `board->vtable[33](refName, &outObj, &outCol, refEntry)` — the Board's factory method
3. This mod patches the `CALL dword ptr [EAX + 0x84]` instruction at 0x0040C4BA to call `universal_factory()` instead
4. `universal_factory()` tries each of the 13 level factory functions in sequence (most inclusive first)
5. The first factory that returns a non-null object wins

## Installation

1. Backup your original `bass.dll` (rename to `bass_real.dll`)
2. Copy this mod's `bass.dll` into the Hamsterball game directory
3. Ensure `bass_real.dll` exists for audio to work

## Usage

Simply add ref names from other levels to your custom MESHWORLD files. For example:
- Add `N:SPEEDCYLINDER` refs to a Warm-up Race custom level → speed boost pads will appear
- Add `N:GEAR` refs to an Intermediate level → rotating gears will appear
- Add `N:BONK` refs to any level → bumpers will appear

## Limitations

- **Sub-mesh dependency**: Some objects require pre-loaded MeshWorld sub-meshes (e.g. TIPPER needs `Levels\Level3-Tipper` loaded in the Board constructor). If the target level didn't load that mesh, the factory may create an object with no visual or silently fail. This is a known limitation — to fully support all refs, the Board constructor would need patching to preload additional meshes.
- **Quality gating**: Some factories check `scene+0x23C` (quality setting) and skip creation on low quality. This is preserved from the original behavior.
- **Audio proxy**: This mod acts as a bass.dll proxy. Missing BASS function exports may cause audio warnings but won't crash the game.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll universal_ref_loader.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Technical Details

- **Hook point**: 0x0040C4BA (6 bytes: `FF 90 84 00 00 00` → `E8 rel32 90`)
- **Factory order**: Master → Impossible → Tower → Expert → Toob → Up → Dizzy → Neon → Wobbly → Sky → Bridge → Odd → Glass
- **Crash tested**: Game survives 35+ seconds on Wine/Xvfb with the hook active

See `docs/REF_LOADING_SYSTEM.md` for the complete reverse-engineering analysis.
