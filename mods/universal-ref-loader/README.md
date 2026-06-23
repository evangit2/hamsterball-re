# Universal Ref Loader — Hamsterball DLL Mod

## What It Does

This mod patches the game's level-object dispatch system to allow **any ref type to be loaded into any level**. Normally, each level only loads refs that its own Board vtable[33] factory recognizes (e.g. SpeedCylinder only in Tower/Up Race, Bonk only in Expert/Master Race, Gears only in Impossible Race).

The mod hooks the vtable[33] dispatch at `0x0040C4BA` (inside `Scene_CreateDynamicObjects` at `0x0040C430`) and replaces it with a universal factory that tries all 13 Arena factories in sequence. Each factory's sub-mesh slot dependencies are checked before calling — factories with unloaded sub-mesh slots are skipped to prevent crashes.

## How to Use

1. Copy `bass.dll` to your Hamsterball game directory (replaces the original bass.dll)
2. Add ref names to MESHWORLD Section 1 in any level file
3. The mod will try all Arena factories to create the object

## Limitations

- **Sub-mesh requirement**: If a ref requires a sub-mesh that wasn't loaded by the current Board constructor, the factory will be skipped (safety check prevents crash, but ref won't be created). For full support, modify the Board constructor to preload additional sub-meshes.
- **Arena vs Race**: The mod tries Arena factories only (they handle the full ref set). Race factories are only used as fallback for PLATFORM/STANDS.
- The mod does not modify sub-mesh loading — it only changes the factory dispatch.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll universal_ref_loader.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Technical Details

- **Hook point**: `0x0040C4BA` (`CALL dword ptr [EAX + 0x84]` → `CALL universal_factory`)
- **Safety**: Each factory's board+0x43xx sub-mesh slots are checked for NULL before calling
- **Factory order**: Most inclusive factories first (Master, Dizzy, Impossible, Expert, Tower, Glass, Toob, Neon, Odd, Sky, Beginner, Intermediate, Wobbly, RaceBase)

See `docs/REF_LOADING_SYSTEM.md` for the complete reverse engineering analysis.
