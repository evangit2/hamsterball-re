# Difficulty Settings Mod (v3)

Difficulty-based **level file redirect** for Hamsterball. Instead of modifying entity names, this mod redirects which MESHWORLD file gets loaded based on tournament difficulty.

## How It Works

### Level File Redirect

The mod hooks `LoadMeshWorld` (0x0045DE30) — the function that takes a level name like `levels\level1` and loads `levels\level1.meshworld`. Before loading, the mod checks:

1. **Are we in Tournament mode?** — Detected by checking `profile+0x10` (party flag) and `profile+0x11` (practice flag). If both are 0, we're in Tournament mode.

2. **What difficulty?** — Reads `App+0x23C` (0=Pipsqueak, 1=Normal, 2=Frenzied).

3. **Redirect based on difficulty:**

| Difficulty | Suffix | Example |
|---|---|---|
| 🟢 Pipsqueak | `-easy` | `levels\level1` → `levels\level1-easy` |
| 🟡 Normal | `-normal` | `levels\level1` → `levels\level1-normal` |
| 🔴 Frenzied! | *(none)* | `levels\level1` (default file) |

4. **Fallback**: If the `-easy` or `-normal` variant doesn't exist, the mod falls back to the default file (no suffix).

**Non-tournament modes** (Practice/Time Trial, Party/2P) always use the default file (no suffix). This matches the vanilla game behavior.

### Entity Spawn Fix

The mod also NOPs the difficulty gates in `Board_Setup` that skip `CreateBadBalls` and `CreateMouseTrap` on Pipsqueak difficulty. This ensures entities appear on all difficulties — they're defined in the level file, just gated out by the game code.

- `0x0041C9E4`: `74 07` → `90 90` (CreateBadBalls gate)
- `0x0041CA05`: `74 07` → `90 90` (CreateMouseTrap gate)

## Creating Difficulty Variants

Create alternate versions of level files with the appropriate suffix:

```
levels\
  Level1.MESHWORLD          ← default (Frenzied!, Practice, Party)
  Level1-easy.MESHWORLD     ← Pipsqueak Tournament
  Level1-normal.MESHWORLD   ← Normal Tournament
  Level2.MESHWORLD
  Level2-easy.MESHWORLD
  Level2-normal.MESHWORLD
  ...
```

If a variant file doesn't exist, the mod falls back to the default file automatically.

## Installation

1. Rename the original `bass.dll` to `bass_real.dll`
2. Copy `bass.dll` from this mod into the game folder
3. Create `-easy` and/or `-normal` variants of level files as needed

## Technical Details

- **Hook target**: `LoadMeshWorld` at 0x0045DE30 (`__thiscall`, ECX=this, stack=levelName)
- **Trampoline**: Copies original 8-byte prologue (MOV EAX,FS:[0] + PUSH -1), JMP back to target+8
- **Tournament detection**: `profile+0x10==0 && profile+0x11==0` (profile at `App+0x220`)
- **Difficulty**: `App+0x23C` (0=Pipsqueak, 1=Normal, 2=Frenzied)
- **Entity gates**: NOP JZ at 0x0041C9E4 and 0x0041CA05

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll difficulty_settings.c \
  -I../shared -lwinmm -Wl,--enable-stdcall-fixup -O2 -static \
  -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```
