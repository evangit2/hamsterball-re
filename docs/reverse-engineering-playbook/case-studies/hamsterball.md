# Case Study: Hamsterball.exe

This case study shows how the generic RE playbook was applied to a real target: Hamsterball (2006, Raptisoft), PE32 i386, MSVC 2003.

## 1. Target Acquisition

- Windows installer and installed directory preserved in `originals/`.
- MD5: `7d25019366b8d7f55906325bd630d7fe`.
- Imports: `D3D8.dll`, `DINPUT8.dll`, `DSOUND.dll`, `BASS.dll`, `USER32.dll`, `KERNEL32.dll`.

## 2. Static Recon

```bash
file Hamsterball.exe               # PE32 executable (GUI) Intel 80386
strings Hamsterball.exe | grep -iE 'direct3d|d3d|dinput|dsound|bass'
pefile summary script               # Image base 0x400000, entry 0xBB4C8
```

Strings revealed:
- Window class `AthenaWindow`
- Level paths `levels\\level1` through `levels\\level8`, arena variants
- Music format `.mo3`
- Font and asset names

## 3. Ghidra Import and Naming

- Imported with default analyzers.
- Started GhidraMCP headless server.
- Applied 975+ renames from `docs/FUNCTION_MAP.md`.
- Final documentation coverage: 100%.

## 4. Subsystem Discovery

| Suspected | Evidence | Confirmed |
|-----------|----------|-----------|
| D3D8 renderer | `Direct3DCreate8` import, `D3D8_RENDERING_PIPELINE.md` | Yes |
| DirectInput8 | `DirectInput8Create` import, `INPUT_SYSTEM.md` | Yes |
| BASS audio | `BASS_Init`, `BASS_MusicLoad`, `.mo3` strings | Yes |
| MESHWORLD levels | Multiple `.meshworld` files, loader strings | Yes |
| Vtable OOP | Constructor vtable assignments, destructors restoring base vtables | Yes |

## 5. Struct Recovery

Key structs recovered using the playbook:

| Struct | Evidence | Size |
|--------|----------|------|
| `Ball` | `Ball_ctor2` allocates 0xC98, vtable 0x4CF3A0 | 0xC98 |
| `Scene` | `Scene_dtor` iterates nested lists | 0x47AC+ |
| `CollisionMesh` | Allocated via operator_new(0xCB0), owner back-pointer +0x10 | 0xCB0 |
| `App` | `App_Initialize_Full` constructs singleton | 0x918+ |

## 6. Verification Story: App+0x5DC

**Initial claim:** `App+0x5DC = Scene* currentScene`

**Verification:**
- Zero raw-C references to `(int)this + 0x5DC` in App functions.
- Raw C in `Ball_Update` showed `Scene+0x5DC` used as a player-data array.
- Correct conclusion: `App→Scene` path remained unverified; only `Ball+0x14 → Scene` was proven.

This demonstrates the rule: *every offset is guilty until proven innocent by raw decompilation.*

## 7. Dynamic Analysis

- Ran original under Wine + Xvfb + d3d8to9 + llvmpipe.
- Observed camera sign discrepancy across levels.
- Confirmed texture behavior differences between loaded and programmatic textures.

## 8. Outputs

- `docs/FUNCTION_MAP.md` — 3,781 named functions.
- `analysis/ghidra/structs/*.h` — C struct definitions.
- `docs/*SYSTEM.md` — subsystem documentation.
- Working D3D8 reimplementation in `reimpl/`.
