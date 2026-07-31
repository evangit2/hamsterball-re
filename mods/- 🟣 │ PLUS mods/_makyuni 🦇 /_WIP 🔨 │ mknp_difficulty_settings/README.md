# Difficulty Settings Mod (v3.1)

Difficulty-based **level file redirect** for Hamsterball. Instead of modifying entity names, this mod redirects which MESHWORLD file gets loaded based on tournament difficulty.

## How It Works

### Level File Redirect

The mod hooks `LoadMeshWorld` (0x0045DE30) — the function that takes a level name like `levels\level1` and loads `levels\level1.meshworld`. Before loading, the mod checks:

1. **Are we in Tournament mode?** — Detected by checking `profile+0x10` (party flag) and `profile+0x11` (practice flag). If both are 0, we're in Tournament mode.

2. **What difficulty?** — Reads `App+0x23C` (0=Pipsqueak, 1=Normal, 2=Frenzied).

3. **Redirect based on difficulty:**

| Mode | Difficulty | Suffix | Example |
|---|---|---|---|
| 🟢 Tournament | Pipsqueak | `-easy` | `levels\level1` → `levels\level1-easy` |
| 🟡 Tournament | Normal | *(none)* | `levels\level1` (default file) |
| 🔴 Tournament | Frenzied! | `-hard` | `levels\level1` → `levels\level1-hard` |
| Practice/Party | — | `-hard` | `levels\level1` → `levels\level1-hard` (fallback to default if missing) |

4. **Fallback**: If the `-easy` or `-hard` variant doesn't exist, the mod falls back to the default file (no suffix).

### Entity Spawn Fix

The mod also NOPs the difficulty gates in `Board_Setup` that skip `CreateBadBalls` and `CreateMouseTrap` on Pipsqueak difficulty. This ensures entities appear on all difficulties — they're defined in the level file, just gated out by the game code.

- `0x0041C9E4`: `74 07` → `90 90` (CreateBadBalls gate)
- `0x0041CA05`: `74 07` → `90 90` (CreateMouseTrap gate)

## Creating Difficulty Variants

Create alternate versions of level files with the appropriate suffix:

```
levels\
  Level1.MESHWORLD          ← default (Normal Tournament, fallback)
  Level1-easy.MESHWORLD     ← Pipsqueak Tournament
  Level1-hard.MESHWORLD     ← Frenzied Tournament / Practice / Party
  Level2.MESHWORLD
  Level2-easy.MESHWORLD
  Level2-hard.MESHWORLD
  ...
```

If a variant file doesn't exist, the mod falls back to the default file automatically.

## Installation

1. Rename the original `mknp_difficulty_settings.dll` to `bass_real.dll`
2. Copy `mknp_difficulty_settings.dll` from this mod into the game folder
3. Create `-easy` and/or `-hard` variants of level files as needed

## Technical Details

- **Hook target**: `LoadMeshWorld` at 0x0045DE30 (`__thiscall`, ECX=this, stack=levelName)
- **Trampoline**: Copies original 8-byte prologue (MOV EAX,FS:[0] + PUSH -1), JMP back to target+8
- **Tournament detection**: `profile+0x10==0 && profile+0x11==0` (profile at `App+0x220`)
- **Difficulty**: `App+0x23C` (0=Pipsqueak, 1=Normal, 2=Frenzied)
- **Entity gates**: NOP JZ at 0x0041C9E4 and 0x0041CA05

## Build

```bash
i686-w64-mingw32-gcc -shared -o mknp_difficulty_settings.dll mknp_difficulty_settings.c \
  -I../shared -lwinmm -Wl,--enable-stdcall-fixup -O2 -static \
  -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```
