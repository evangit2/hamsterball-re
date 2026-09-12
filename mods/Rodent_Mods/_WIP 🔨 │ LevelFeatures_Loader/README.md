# LevelFeatures_Loader — universal cross-level objects for Hamsterball

A `bass.dll` proxy mod. Replaces all 15 per-level board constructors **and**
4 vtable slots with universal handlers, so any level's objects (bridges,
tippers, bumpers, windmills, …) work on any other level. Fully S1-driven:
drop a `.MESHWORLD` into any slot and its `N:`/`E:` refs bring their own
meshes and behaviors — no config editing.

Game version: V3.6c. Load mechanism: `bass.dll` proxy (rename original to
`bass_real.dll`).

## How it works

**Universal constructor.** A 6-byte JMP detour on the scene-loader call
(`CALL [EDX+0x48]` at `0x4273E0`) routes every level through
`UniversalConstructor`, which runs the same 4 steps every native loader
runs: `Level_MeshWorldCtor` → `Level_RenderCtor` → `Level_InitScene` →
per-level tail replication (Tower pennants, Neon glow lights, Up VAC tubes,
Sky magnifier) → `UniversalPostSetup` → `Board_Setup` (`vtable+0x80`).

**Per-board extension heap (Option B).** Boards stay vanilla-sized. All
mod state lives in a `0xC000` heap block per board (`g_extMap`, no board
mirrors). `ScanS1AndAutoEnable` reads the loaded file's S1 refs after
`MeshWorldCtor` and sets per-board `FEAT_*` flags, so file-swapped levels
just work.

**Vtable patching** (`InstallVtablePatches`, ~1s after load, before first
level):

| Slot | Offset | Handler | Strategy |
|------|--------|---------|----------|
| 1 | +0x04 | `UniversalBoardUpdate` | Calls shared `Scene_Update` (0x419C00) first, then feature blocks |
| 19 | +0x4C | `UniversalRaceState` | Calls the saved original **or** the shared base exactly once, then feature blocks (Beginner skipped — `Feature_BumperDecay` owns it; Sky skipped when ext feat active) |
| 24 | +0x60 | `UniversalRender` (RET-4 thunk) | Only the 6 wrapper levels {2,5,10,12,13,14}; the other 9 keep native shared render (all 15 natives are `(board,gfx)` RET 4, verified by disasm) |
| 29 | +0x74 | `UniversalDispatchCollision` | Reimplements all 62 collision events inline (gated on per-board flags built once at load), then calls global `DispatchCollisionEvents` |
| 33 | +0x84 | `UniversalCreateDynamicObjects` | Matches S1 names, calls entity ctors directly (no delegation — originals only know their own level's names) |

Sub-meshes lazy-load via `S1EnsureMeshWorld` inside CreateDynamicObjects —
no static preload. `Tournament_AdvanceRace` (0x427080) is hooked with an
ECX-preserving wrapper to free the outgoing board's ext block.

## Feature blocks

| Feature | Levels | Source | Description |
|---------|--------|--------|-------------|
| `FEAT_BRIDGE_ANIM` | 3, 14 | Intermediate (0x41CC90) | 4-state bridge tilt machine |
| `FEAT_SWIRL` | 4, 14 | Dizzy (0x41D510), Master (0x420DA0) | Swirl zones + tar bubbles |
| `FEAT_WINDMILL` | 5 | Tower (0x41E760) | Windmill rotation + speed states |
| `FEAT_BADBALL` | 9 | Odd (0x41EE80) | Timer-based BadBall spawner (dormant-faithful: native flag is pinned 0 in vanilla) |
| `FEAT_BUMPER_DECAY` | 2, 10, 14 | Beginner/Toob/Master | Bumper "lit" float decay |
| `FEAT_NEON_CAM` | 7 | Neon (0x424790) | Ball-following glow lights (S3-gap gated, see below) |
| `FEAT_SKY_POPCYL` | 13 | Sky (0x41FC90) | Random popcylinder activation |

## Per-name pennants

Name a flag mesh `FLAG*` for a rectangle, `PENNANT*` for a triangle — on
any level, in any slot (`src/lf_09_pennants.c`). The game natively runs
every flag through one shared waver (all-rectangle, or all-pennant on
Tower); the mod builds a second pennant waver and routes each flag by name
at render time. Check `lfdebug.log` for `Pennant:` lines to confirm pickup.

## Neon glow gate

Neon follower lights build **only** when the level MESHWORLD has no S3
light of its own (`src/lf_10_neon.c`). Stock Neon (`LevelDark`, S3=0) gets
followers; every other stock race keeps its native lights. Custom authors
opt out by adding any S3 light.

## RaceFiles.txt

Maps `Race 1`..`Race 15` to a file — swap any race to any `.MESHWORLD`
without renaming files:

```ini
Race 1: Level1          # WarmUp slot loads levels\level1
Race 4: Level3          # Dizzy file in the WarmUp slot (always unlocked in Time Trials)
```

Bare names auto-prepend `levels\`; `.MESHWORLD` extension auto-stripped.
Re-read on every level load. This is the **only** config file —
`LevelFeatures.txt` (v11) and `LevelData.txt` (v10) are deprecated and no
longer read; S1 `N:`/`E:` refs are the source of truth.

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy `bass.dll` and `RaceFiles.txt` from the zip into the game folder

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll LevelFeatures.c \
  bass.def -lwinmm -static-libgcc -Wl,--enable-stdcall-fixup -O2
```

Source lives in `src/` as 11 chapters (one translation unit —
`LevelFeatures.c` only `#include`s them, output is still one `bass.dll`):

| Chapter | Contents |
|---|---|
| `lf_00_core.c` | RVAs, offsets, ext heap, globals, string/level/racefile helpers |
| `lf_01_bumpers.c` | Bumper physics constants, game fn pointers, mesh path table |
| `lf_02_ctor.c` | UniversalBoardCtorLogic + install thunks |
| `lf_03_features.c` | InitBridge + Feature blocks (bridge/swirl/windmill/…) |
| `lf_04_render_update.c` | Render + BoardUpdate + RaceState |
| `lf_05_create.c` | S1Ensure + UniversalCreateDynamicObjects |
| `lf_06_collision.c` | Collision flags + UniversalDispatchCollision |
| `lf_07_postscan.c` | PostSetup + S1/collision scans |
| `lf_08_hooksmain.c` | Constructor + hooks + DebugLog + vtable + DllMain |
| `lf_09_pennants.c` | Per-name pennant routing (`FLAG*`=rect, `PENNANT*`=triangle) |
| `lf_10_neon.c` | Neon glow follower lights, S3-gap gated |

## Files

- `bass.dll` — compiled mod
- `RaceFiles.txt` — slot→file map (the only config file)
- `LevelFeatures.txt` — **deprecated**, no longer read (kept for reference)
- `LevelFeatures.xml` — reference catalog of injectable objects (docs only)
- `bass.def` — export definitions for the bass.dll proxy
- `LevelFeatures_Loader.zip` — release artifact (`bass.dll` + `README.md`)

## History

- **v13.1** — Neon scene-loader tail (Step 6c) with S3-gap gate
- **v13** — Option B: per-board ext heap, vanilla-sized boards, S1 feat scan
- **v12.3** — Tower/Sky render split
- **v12** — Level-folder sub-mesh scan
- **v11** — S1-driven collisions, `LevelFeatures.txt` deprecated
- **v9** — `RaceFiles.txt` swappable races
- **v8** — TURRET Timer-vtable fix, dedicated mesh slots

## Testing notes

- Crash test = launch on Wine/Xvfb, survive 35s past title (catches bad
hooks/stack corruption). Wine renders black on llvmpipe and DirectInput
ignores synthetic keys, so level-load paths need real Windows testing.
- Debug log: `lfdebug.log` next to the config (INIT line lists paths).
