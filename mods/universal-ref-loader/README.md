# Universal Ref Loader — Hamsterball DLL Mod v4

## What It Does

This mod patches the game's level-object dispatch system to allow **any object ref to be loaded into any level**. Normally, each level only loads refs that its own Board vtable[33] factory recognizes (e.g. SpeedCylinder only in Up levels, Bonk only in Expert/Master levels, Gears only in Impossible level).

The mod hooks the vtable[33] dispatch at `0x0040C4BA` (inside `Scene_CreateDynamicObjects` at `0x40C430`) and replaces it with a universal factory that:

1. **Tries the original factory first** — preserves normal behavior for all existing refs
2. **Falls through to all 13 Arena factories** if the original didn't handle the ref
3. **JIT mesh injection** — if a factory needs a board mesh slot that's NULL (not loaded by the current level), loads the mesh from disk on the fly
4. **Clones static-mesh objects** (WATERWHEEL, SWIRL, BRIDGE-base) via `Level_CloneTree` for multi-instance support
5. **Bypasses difficulty gates** — temporarily sets `App+0x23C = 1` (Normal) for gated objects (TIPPER, BONK, BLOCKDAWG, GLUEBIE, FAN, SAWBLADE, MACE)
6. **Safety checks board slots** — skips factories whose required mesh slots can't be filled (prevents crashes)

## How to Use

1. Rename original `bass.dll` to `bass_real.dll` (for audio passthrough)
2. Copy this mod's `bass.dll` to your Hamsterball game directory
3. Add ref names to MESHWORLD Section 1 in any level file
4. The mod will try all Arena factories to create the object, loading meshes from disk as needed

## JIT Mesh Injection

When a factory reads a board+0x4xxx slot that's NULL (because the current level's Board constructor didn't load that mesh), the mod:

1. Looks up the board offset in the mesh database (maps offsets → file paths)
2. Calls `MeshWorld_ctor(mem, d3dDevice, path)` to load the mesh from disk
3. Optionally calls `CollisionLevel_ctorWithLevel(mem, mesh)` for collision
4. Writes the loaded mesh pointer into the board slot
5. Calls the factory (which reads the slot)
6. Restores the original NULL value after the factory returns

Meshes are cached so each file is only loaded once per session.

### Mesh Database (verified from decompiled board constructors)

| Board Slot | Mesh File | Collision? | Used By |
|-----------|----------|-----------|---------|
| +0x436C | Level3-Tipper / Level4-Catapult / LevelImpossible-Looper | +0x4370 | Dizzy, Tower, Impossible |
| +0x4370 | Level4-Drawbridge / LevelImpossible-Gear | — | Tower, Impossible |
| +0x4374 | Level3-Gluebie / LevelImpossible-BigGear | — | Dizzy, Impossible |
| +0x4378 | Level4-Mace / Level5-Bridge / LevelImpossible-Rotator | — | Tower, Expert, Impossible |
| +0x437C | Level4-Windmill / LevelImpossible-Pendulum | — | Tower, Impossible |
| +0x4394 | Level3-Tipper | +0x4398 | Master |
| +0x4BA8 | Level3-WaterWheel | +0x4BAC | Dizzy |
| +0x4BC4 | Level3-Swirl | +0x4BC8 | Dizzy |
| +0x4784 | LevelUp-Lifter | — | Up |
| +0x4788 | LevelUp-SpeedCylinder | — | Up |
| +0x478C | LevelUp-Button | — | Up |
| +0x47E0 | Level8-Spinny | — | Toob/Odd/Glass |
| +0x5410 | Level10-Bridge1 | — | Master |
| +0x5414 | Level10-Bridge2 | — | Master |
| +0x5420 | Level9-PopCylinder1 | — | Master, Sky |
| +0x5840 | Level8-BlockDawg1 | — | Master, Toob |
| +0x5844 | Level8-BlockDawg2 | — | Master, Toob |
| +0x5848 | Level4-Catapult | — | Master |
| +0x607C | Level3-Gluebie | — | Master |

## Verified Features

| Feature | Status | Details |
|---------|--------|---------|
| Hook point | ✅ Verified | `CALL [EAX+0x84]` at 0x0040C4BA, 6 bytes `FF 90 84 00 00 00` |
| Factory addresses | ✅ Verified | All 30 factories (15 Arena + 15 Race) confirmed via vtable[33] reads |
| Level_CloneTree | ✅ Verified | `__thiscall` at 0x466060, allocs 0x10D0, recursive spatial tree clone |
| MeshWorld_ctor | ✅ Verified | `__thiscall` at 0x461510, params: (mem, d3dDevice, path) |
| CollisionLevel_ctor | ✅ Verified | `__thiscall` at 0x465080, params: (mem, sourceMesh) |
| Board slot offsets | ✅ Verified | All offsets cross-referenced from decompiled factory code |
| Difficulty gate | ✅ Verified | `board+0x878 → App+0x23C != 0` confirmed in 7 factory handlers |
| D3D device path | ✅ Verified | `*(board+0x878) + 0x174` (App+0x174) |
| BASS proxy | ✅ v3 pattern | Lazy loader, no DllMain deadlock, stubs if bass_real.dll missing |
| Crash test | ✅ Passed | 14.12s runtime, no crash (hbtestd) |

## Static-Mesh Cloning

Objects like WATERWHEEL and SWIRL return the same board slot pointer for every ref — only one instance renders. This mod creates a fresh `MeshWorld` via `MeshWorld_ctor` for each ref instance:

| Object | Board Slot | Instance Method |
|--------|-----------|-----------------|
| WATERWHEEL | +0x4BA8 | ✅ Fresh MeshWorld per ref |
| SWIRL | +0x4BC4 | ✅ Fresh MeshWorld per ref |
| BRIDGE (base) | +0x436C | ✅ Fresh MeshWorld per ref |
| WINDMILL | +0x437C | ⚠️ Not handled (complex: creates CollisionLevel + attaches) |

**Note:** v3 used `Level_CloneTree` (0x466060) which created a `Level` (base class)
with an uninitialized world matrix — caused NULL pointer crash in matrix inverse
function (0x49B4E7) during Draw. v4 replaces this with `MeshWorld_ctor` (0x461510)
so each instance gets its own valid vtable, world matrix, and render state.

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

- **WINDMILL**: Returns static mesh + creates CollisionLevel with attach — not cloned yet (complex multi-object creation).
- **N:/E: prefixed refs**: Handled by the original factory's N:/E: handler, not by the universal dispatch.
- **Slot ambiguity**: Some board offsets map to different mesh files on different levels (e.g. 0x436C = Tipper on Dizzy but Catapult on Tower). The mesh DB tries all entries; the first that loads successfully is used.

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll universal_ref_loader.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Technical Details

- **Hook point**: `0x0040C4BA` (`CALL [EAX+0x84]` → `CALL universal_factory` + NOP)
- **Calling convention**: `__thiscall` (ECX=board), 4 stack args
- **JIT mesh loading**: `MeshWorld_ctor` (0x461510) + `CollisionLevel_ctorWithLevel` (0x465080)
- **D3D device**: `*(board+0x878) + 0x174` (App+0x174)
- **BASS proxy**: v3 lazy loader pattern (LoadLibraryA on first BASS call, not in DllMain)
- **All addresses verified** via GhidraMCP decompilation + vtable memory reads (June 2026)

See `docs/UNIVERSAL_REF_LOADER_DESIGN.md` for the complete reverse engineering analysis.
