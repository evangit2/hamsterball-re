# Universal Ref Loader — Hamsterball DLL Mod v2

## What It Does

This mod patches the game's level-object dispatch system to allow **any object ref to be loaded into any level**. Normally, each level only loads refs that its own Board vtable[33] factory recognizes (e.g. SpeedCylinder only in Up levels, Bonk only in Expert/Master levels, Gears only in Impossible level).

The mod hooks the vtable[33] dispatch at `0x0040C4BA` (inside `Scene_CreateDynamicObjects` at `0x40C430`) and replaces it with a universal factory that:

1. **Tries the original factory first** — preserves normal behavior for all existing refs
2. **Falls through to all 13 Arena factories** if the original didn't handle the ref
3. **Clones static-mesh objects** (WATERWHEEL, SWIRL, BRIDGE-base) via `Level_CloneTree` for multi-instance support
4. **Bypasses difficulty gates** — temporarily sets `App+0x23C = 1` (Normal) for gated objects (TIPPER, BONK, BLOCKDAWG, GLUEBIE, FAN, SAWBLADE, MACE)
5. **Safety checks board slots** — skips factories whose required mesh slots are NULL (prevents crashes)

## How to Use

1. Rename original `bass.dll` to `bass_real.dll` (for audio passthrough)
2. Copy this mod's `bass.dll` to your Hamsterball game directory
3. Add ref names to MESHWORLD Section 1 in any level file
4. The mod will try all Arena factories to create the object

## Verified Features

| Feature | Status | Details |
|---------|--------|---------|
| Hook point | ✅ Verified | `CALL [EAX+0x84]` at 0x0040C4BA, 6 bytes `FF 90 84 00 00 00` |
| Factory addresses | ✅ Verified | All 30 factories (15 Arena + 15 Race) confirmed via vtable[33] reads |
| Level_CloneTree | ✅ Verified | `__thiscall` at 0x466060, allocs 0x10D0, recursive spatial tree clone |
| Board slot offsets | ✅ Verified | All offsets cross-referenced from decompiled factory code |
| Difficulty gate | ✅ Verified | `board+0x878 → App+0x23C != 0` confirmed in 7 factory handlers |
| BASS proxy | ✅ v3 pattern | Lazy loader, no DllMain deadlock, stubs if bass_real.dll missing |
| Crash test | ✅ Passed | 13.65s runtime, no crash (hbtestd) |

## Static-Mesh Cloning

Objects like WATERWHEEL and SWIRL return the same board slot pointer for every ref — only one instance renders. This mod calls `Level_CloneTree` to create independent copies:

| Object | Board Slot | Clone Status |
|--------|-----------|-------------|
| WATERWHEEL | +0x4BA8 | ✅ Cloned |
| SWIRL | +0x4BC4 | ✅ Cloned |
| BRIDGE (base) | +0x436C | ✅ Cloned |
| WINDMILL | +0x437C | ⚠️ Not cloned (complex: creates CollisionLevel + attaches) |

## Factory Dispatch Order

Factories are tried in order of inclusiveness (most ref types first):

1. **Expert** — BONK, FAN, SAWBLADE, BRIDGE, JUDGE, BELL (no mesh deps, self-loading)
2. **Wobbly** — SMASHER1, SMASHER2 (configuring only)
3. **Master** — BRIDGE, TIPPER, BONK, BBRIDGE1-2, POPCYLINDER, BLOCKDAWG1-2, CATAPULT, GLUEBIE
4. **Tower** — CATAPULT, MACE, DRAWBRIDGE, WINDMILL, TRAPDOOR, CHOMPER, TURRET
5. **Impossible** — LOOPER, GEAR, BIGGEAR, ROTATOR, PENDULUM
6. **Up** — LIFTER, SPEEDCYLINDER, TIMEBUTTON
7. **Dizzy** — TIPPER, WATERWHEEL, SWIRL, GLUEBIE
8. **Beginner** — BRIDGE (base)
9. **Neon, Odd, Toob, Glass, Sky** — level-specific objects

## Limitations

- **Mesh dependency**: If a ref requires a mesh that wasn't loaded by the current Board constructor, the factory is skipped (safety check prevents crash, but ref won't be created). Full mesh injection (JIT loading from disk) is planned for a future version.
- **WINDMILL**: Returns static mesh + creates CollisionLevel with attach — not cloned yet (complex multi-object creation).
- **N:/E: prefixed refs**: Handled by the original factory's N:/E: handler, not by the universal dispatch.
- The mod does not modify Board constructors to preload additional meshes — it only changes the factory dispatch and adds safety + cloning.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll universal_ref_loader.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Technical Details

- **Hook point**: `0x0040C4BA` (`CALL [EAX+0x84]` → `CALL universal_factory` + NOP)
- **Calling convention**: `__thiscall` (ECX=board), 4 stack args
- **BASS proxy**: v3 lazy loader pattern (LoadLibraryA on first BASS call, not in DllMain)
- **All addresses verified** via GhidraMCP decompilation + vtable memory reads (June 2026)

See `docs/UNIVERSAL_REF_LOADER_DESIGN.md` for the complete reverse engineering analysis.
