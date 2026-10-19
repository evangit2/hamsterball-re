# Custom Name Strips Mod

Custom per-level RGBA colors for the horizontal strip displayed behind the Race/Arena name at the beginning of each race.

## Installation

1. Rename original `mknp_custom_name_strips.dll` → `bass_real.dll` in the Hamsterball game folder
2. Copy the mod `mknp_custom_name_strips.dll` and `mknp_custom_name_strips.txt` into the game folder
3. Launch Hamsterball — the mod auto-creates `mknp_custom_name_strips.txt` on first run if missing

## Config File

The mod reads `mknp_custom_name_strips.txt` next to `mknp_custom_name_strips.dll`. Edit it at runtime — changes apply within 2 seconds.

### Format

```
; Per-level RGBA colors for the name strip
; Values are floats (0.0 to 1.0)
; Original game: R=1.0, G=1.0, B=0.0, A=0.75 (yellow)

Level 1 =
R = 1.0, G = 1.0, B = 1.0, A = 1.0

Level 2 =
R = 1.0, G = 1.0, B = 1.0, A = 1.0
...
Level 15 =
R = 1.0, G = 1.0, B = 1.0, A = 1.0

Arena 1 =
R = 1.0, G = 1.0, B = 1.0, A = 1.0
...
Arena 15 =
R = 1.0, G = 1.0, B = 1.0, A = 1.0
```

### Level Mapping

| Number | Race | Arena |
|--------|------|-------|
| 1 | Warm-Up Race | Warm-Up Arena |
| 2 | Beginner Race | Beginner Arena |
| 3 | Intermediate Race | Intermediate Arena |
| 4 | Dizzy Race | Dizzy Arena |
| 5 | Tower Race | Tower Arena |
| 6 | Up Race | Up Arena |
| 7 | Neon Race | Neon Arena |
| 8 | Expert Race | Expert Arena |
| 9 | Odd Race | Odd Arena |
| 10 | Toob Race | Toob Arena |
| 11 | Wobbly Race | Wobbly Arena |
| 12 | Glass Race | Glass Arena |
| 13 | Sky Race | Sky Arena |
| 14 | Master Race | Master Arena |
| 15 | Impossible Race | Impossible Arena |

## How It Works

### The Strip

At the start of each race, the game displays the race/arena name with a colored horizontal strip behind it. The strip color is set by 4 `PUSH` instructions at addresses 0x41B763–0x41B76B in function `FUN_0041B710` (Scene_DrawNameStrip):

| Address | Instruction | Channel | Original Value |
|---------|------------|---------|----------------|
| 0x41B763 | `push 0x3F400000` | Alpha | 0.75 |
| 0x41B768 | `push 0` (imm8) | Blue | 0.0 |
| 0x41B769 | `push ebp` | Green | 1.0 |
| 0x41B76B | `push ebp` | Red | 1.0 |

These produce the original yellow strip (R=1.0, G=1.0, B=0.0, A=0.75).

### The Mod

Since 3 of the 4 push instructions are not 5-byte `push imm32` (one is `push 0` 2-byte, two are `push ebp` 1-byte), a code cave is used:

1. A 30-byte code cave is allocated via `VirtualAlloc`
2. The cave contains 4× `push imm32` (all 5-byte, patchable) + `call Matrix_Scale4x4` + `jmp` back
3. The original 14-byte push sequence at 0x41B763 is replaced with `JMP cave` + NOPs
4. A background thread reads the level name from `board+0x29B4` and patches the cave's float operands

### Level Detection

The mod reads the race/arena name string from `board+0x29B4` and matches it against level keywords:

| Keyword | Level |
|---------|-------|
| WARM | Warm-Up |
| BEGINNER | Beginner |
| INTERMED | Intermediate |
| DIZZY | Dizzy |
| TOWER | Tower |
| UP | Up |
| NEON | Neon |
| EXPERT | Expert |
| ODD | Odd |
| TOOB | Toob |
| WOBBLY | Wobbly |
| GLASS | Glass |
| SKY | Sky |
| MASTER | Master |
| IMPOSSIBLE | Impossible |

Arena vs Race is determined by checking if "ARENA" appears in the name string.

## Build

```bash
i686-w64-mingw32-gcc -shared -o mknp_custom_name_strips.dll mknp_custom_name_strips.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```
