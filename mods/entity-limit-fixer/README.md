# EntityLimitFixer v3

Prevents freezes/crashes when spawning many entities (8-balls, clones) in arenas.

## Root Cause (verified via GhidraMCP decompilation)

`Ball_AI_ChaseNearest` (0x408390) contains **two O(N²) loops** that iterate the entire ball list (`Scene+0x29D4`, count at `Scene+0x29D8`) per AI ball per frame:

| Loop | Address | Purpose | Complexity |
|------|---------|---------|------------|
| Loop 1 | 0x4083D9 | "Near ball bonus" — awards score to nearby balls | O(N) per AI ball |
| Loop 2 | 0x408548 | "Find nearest target" — 6 flag checks + distance per ball | O(N) per AI ball |

With N balls, each AI ball does 2N distance calculations → **2N² total per frame**.
- N=30: 1,800 calcs/frame → mild slowdown
- N=50: 5,000 calcs/frame → severe freeze
- N=100+: frame budget overflow → permanent freeze

Additionally, `Ball_Update` (0x405E00) allocates per-frame objects:
- Trail particle: `operator_new(0x28)` at 0x405ECB — 40 bytes per ball per frame
- CollisionNode: `operator_new(0x14)` at 0x4068F5 — 20 bytes per ball per frame

At high entity counts, these allocations fragment the heap and eventually fail → crash.

## Fix

4 code caves that check `Scene+0x29D8` (ball count) against `MAX_BALLS` (default 30):

1. **Patch 1 (0x4083D9)**: Skip AI Loop 1 (near ball bonus) → JMP 0x4084F5
2. **Patch 2 (0x408548)**: Skip AI Loop 2 (find nearest target) → JMP 0x408663, EBX=0
3. **Patch 3 (0x405ECB)**: Skip trail particle alloc → return NULL (game handles gracefully)
4. **Patch 4 (0x4068F5)**: Skip CollisionNode alloc → return NULL (game handles gracefully)

When ball count ≤ MAX_BALLS, all patches execute original code unchanged.
When ball count > MAX_BALLS, loops are skipped and allocations return NULL.

**Balls still exist, render, and have physics** — they just skip AI target search and per-frame allocations when overcrowded. 8-balls wander instead of chasing.

## Verification

All addresses verified against GhidraMCP disassembly:
- `Ball_AI_ChaseNearest` @ 0x408390: decompiled, both loops confirmed
- `Ball_Update` @ 0x405E00: disassembled 600 instructions, alloc sites confirmed
- `AthenaList_NextIndex` @ 0x004532B0: confirmed via call sites
- `operator_new` @ 0x004BA57B: confirmed via call sites
- Skip targets (0x4084F5, 0x408663, 0x00405ED5, 0x004068FF): verified in disassembly
- NULL paths: game already handles NULL returns (JZ to skip init) at 0x406922 and 0x405EF9

## Usage

1. Open Cheat Engine, attach to Hamsterball.exe
2. Load EntityLimitFixer.CEA
3. Enable the script
4. Spawn as many 8-balls/clones as you want

## Configuration

Change `MAX_BALLS` at the top of the script:
- `20` = conservative (very stable)
- `30` = balanced (default, handles most scenarios)
- `50` = aggressive (may still slow down with 50+ balls)
