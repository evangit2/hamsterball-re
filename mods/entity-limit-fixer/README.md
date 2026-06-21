# EntityLimitFixer v4

Prevents freezes AND crashes when spawning many entities (8-balls, clones) in arenas.

## What changed from v3

**v3 still crashed because** it only patched AI loops and allocations — but the crash comes from `operator_new` throwing `bad_alloc` when the heap exhausts. The `bad_alloc` C++ exception (`__CxxThrowException`) unwinds the stack and crashes the game because Hamsterball's SEH handlers can't recover.

**v4 fixes the crash at the source:** patches `operator_new` to return NULL instead of throwing, then caps allocations at the call sites.

## Root Cause (full decompilation trace)

### Crash path
```
operator_new(0x4BA57B)
  → _malloc returns NULL (heap exhaustion)
  → __callnewh (0x4BD9CF) — no new handler installed, returns 0
  → CRT_ThrowBadAlloc (0x4BA69D)
  → __CxxThrowException → CRASH
```

Each ball allocates: **~4.6KB** (Ball_ctor2: 0xC60 + CollisionMesh: 0xCB0).
Per-frame per-ball: trail (0x28) + CollisionNode (0x14) + Material (0x68) + Mesh_FindClosestCollision (creates+frees a SpatialTree every call).
At 50+ balls, heap fragments and exhausts → `bad_alloc` → crash.

### Freeze path
Each ball per frame calls `Ball_Update` (0x405E00) which does 5+ heap operations:
- `operator_new(0x28)` trail particle at 0x405ECB
- `operator_new(0x14)` CollisionNode at 0x4068F5
- `Mesh_FindClosestCollision` at 0x40651F (allocates+frees SpatialTree)
- `AthenaList_Remove` at 0x406381 (`_malloc(N*4)` + copy + `_free` + `_realloc`)
- `AthenaList_Append` at 0x40671A (`_realloc` every call = O(N) copy)

Plus `Ball_AI_ChaseNearest` (0x408390) has two O(N²) loops per AI ball.

## Fix — 4 patches (all verified via GhidraMCP)

| # | Address | What | How |
|---|---------|------|-----|
| 1 | 0x4BA58D | operator_new crash | Replace `CALL CRT_ThrowBadAlloc` with `XOR EAX,EAX; POP ESI; RET` — return NULL instead of throwing |
| 2 | 0x405ECB | Trail particle alloc | When ball count > MAX_BALLS, return NULL (game already handles via `CMP EAX,EBX; JZ skip`) |
| 3 | 0x4068F5 | CollisionNode alloc | Same — return NULL when overcrowded (game handles via `JZ 0x406922`) |
| 4a | 0x4083D9 | AI Loop 1 (near ball bonus) | Skip O(N²) loop when count > MAX_BALLS → JMP 0x4084F5 |
| 4b | 0x408548 | AI Loop 2 (find nearest target) | Skip O(N²) loop when count > MAX_BALLS → JMP 0x408663, EBX=0 |

**Key insight:** All game allocation sites already have NULL checks (`CMP EAX,EBX; JZ skip_init`). The crash only happens because `operator_new` *throws* instead of returning NULL. Patch 1 fixes the crash; patches 2-4 prevent the freeze.

## Usage

1. Open Cheat Engine, attach to Hamsterball.exe
2. Load EntityLimitFixer.CEA
3. Enable the script
4. Spawn as many 8-balls/clones as you want — no crash, no freeze

## Configuration

Change `MAX_BALLS` at top of script:
- `20` = conservative (very stable)
- `30` = balanced (default)
- `50` = aggressive (may slow down with 50+ balls, but won't crash)
- `99` = max (won't crash, may stutter at very high counts)
