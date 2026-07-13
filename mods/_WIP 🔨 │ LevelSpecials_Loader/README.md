# LevelSpecials_Loader (v3)

Universal cross-level object injection for Hamsterball. Replaces all 15 per-level constructors with a single universal constructor, then injects config-driven features (bumpers, etc.) into any level.

## How It Works

The mod intercepts `Tournament_AdvanceRace` (0x00427080) at the point where it calls the per-level `Scene_LoadLevel*` function via `CALL [EDX+0x48]` (vtable[18]). Instead of letting the original per-level constructor run, a **universal constructor** takes over and performs the same 4 steps every level does:

1. `operator_new(0x10D0)` → `Level_MeshWorldCtor(mem, gfx, meshPath)` → `board+0x8AC`
2. `operator_new(0x10D0)` → `Level_RenderCtor(mem, meshWorld)` → `board+0x8B0`
3. `Level_InitScene(board)`
4. `board->vtable[0x80]()` = `Board_Setup`
5. `UniversalPostSetup(board)` — config-driven features

The per-level `Scene_LoadLevel*` functions **never run**. The universal constructor reads the race index from `[ESI+0x8]` and looks up the correct mesh path from a table.

### Three hooks

| Hook | Address | Description |
|------|---------|-------------|
| **Alloc size patch** | 15 sites in `Tournament_AdvanceRace` (0x27109–0x273A5) | Patches all 15 `PUSH imm32` allocation sizes to `0xA2F8` (union of all board struct sizes), so every level gets enough memory for all possible objects |
| **Universal constructor** | 0x4273E0 (`CALL [EDX+0x48]`) | 6-byte JMP detour. Naked thunk reads race index from `[ESI+0x8]`, calls `UniversalConstructor(board, raceIndex)`. Trampoline executes only `MOV ECX,[ESI+4]` + JMP back — original constructor is skipped entirely |
| **Collision hook** | 0x40C5D0 (`DispatchCollisionEvents`) | 8-byte trampoline. Intercepts all collision events, checks for `N:BUMPER` mesh names, applies bounce physics (vel×4.0, clamp 5.0–10.0, zero Y, 3D sound at ball position) |

### Level identification

The mod identifies the current level by reading the board's vtable pointer (`*(DWORD*)board`) and matching it against a table of 15 known level vtable addresses. This is more reliable than string matching and works at any point during gameplay.

## Config file

`LevelSpecials.txt` (next to `bass.dll`) controls which features are active per level:

```ini
# Level numbers: 1=WarmUp 2=Beginner 3=Intermediate 4=Dizzy 5=Tower
#   6=Up 7=Neon 8=Expert 9=Odd 10=Toob 11=Wobbly 12=Glass 13=Sky
#   14=Master 15=Impossible

[BUMPERS]
N:BUMPER1 = 2 5 8
N:BUMPER2 = 1 3
N:BUMPER3 = ()
...
N:BUMPER8 = ()
```

List level numbers after `=` to enable bumpers for those levels. Empty `()` means disabled. If any bumper line lists a level, all 8 bumper slots are initialized for that level via `Scene_CollectByNameFilter`.

The config is re-read every time a level loads (in `UniversalPostSetup`), so changes take effect on next race without restarting the game.

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy `bass.dll` and `LevelSpecials.txt` from this mod into the game folder

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll LevelSpecials.c \
  -lwinmm -static-libgcc -Wl,--enable-stdcall-fixup -O2
```

## Files

- `bass.dll` — compiled mod (rename original bass.dll to bass_real.dll)
- `LevelSpecials.c` — source code
- `LevelSpecials.txt` — config file
- `LevelSpecials.xml` — reference catalog of all injectable objects (documentation only, not parsed)
- `bass.def` — export definitions for bass.dll proxy

## Compatibility

- Game version: V3.6.c
- Load mechanism: bass.dll proxy
- The mod patches 15 allocation sites, 1 constructor call, and 1 collision handler. All patches verify original bytes before applying.

## Architecture notes

- **Replacement, not augmentation** — the universal constructor fully replaces all 15 per-level `Scene_LoadLevel*` functions. The original constructors never execute.
- **Union sizing** — all 15 allocation sites are patched to `0xA2F8` (the largest board struct size across all levels), so any level can hold any combination of objects without heap corruption.
- **Config-driven** — features are controlled by `LevelSpecials.txt`, not hardcoded. Adding new feature types (beyond bumpers) means adding new config sections and handlers in `UniversalPostSetup`.
- **Bumper physics** match native game behavior exactly: velocity ×4.0, clamped to 5.0–10.0, Y velocity zeroed, sound played at ball position via `Sound_Play3D`.
