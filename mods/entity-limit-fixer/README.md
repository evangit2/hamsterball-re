# EntityLimitFixer v6

Prevents freezes AND crashes when spawning many entities (8-balls, clones) in arenas.

## What changed from v4

**v4 still froze** because it only patched:
- operator_new (crash fix)
- AI loops (partial — only 8-balls)
- Respawn throttle (too aggressive — every 3rd frame)

**v6 fixes the freeze at the source** by patching the THREE compounding per-frame costs:

### The Freeze Root Cause (traced via GhidraMCP)

```
Per frame at 30 balls (10 fallen clones, 16 respawn points):

  1. Ball_FindClosestRespawnPoint (called every frame per fallen clone):
     → iterates ALL 16 respawn points
     → for EACH: calls Mesh_FindClosestCollision at 0x405C46
       (builds SpatialTree + CollisionMesh + traverses geometry + frees)
     → 10 × 16 = 160 spatial tree builds per frame

  2. Ball_Update (called per ball per frame):
     → calls Mesh_FindClosestCollision at 0x40651F (floor collision)
     → calls Mesh_FindClosestCollision at 0x407557 (wall collision)
     → 20 × 2 = 40 spatial tree builds per frame

  3. Ball_AI_ChaseNearest (called per 8-ball per frame):
     → two O(N) loops scanning ALL balls = O(N²) total
     → 30 × 30 = 900 iterations per frame

  TOTAL: ~200 spatial tree builds + thousands of allocs per frame
  → game thread takes >16ms → FREEZE
```

## v6 Patches (all verified via GhidraMCP disassembly)

| # | Address | What | How |
|---|---------|------|-----|
| 1 | 0x4BA58D | operator_new crash | Replace `CALL CRT_ThrowBadAlloc` with `XOR EAX,EAX; POP ESI; RET` — return NULL instead of throwing |
| 2a | 0x40651F | Mesh_FindClosestCollision #1 in Ball_Update | Cave: if ball count > MAX_BALLS, skip call and write 99999 (no collision) |
| 2b | 0x407557 | Mesh_FindClosestCollision #2 in Ball_Update | Same cave pattern — skip when too many balls |
| 3 | 0x405C46 | Mesh_FindClosestCollision in RespawnPoint | NOP the CALL — pick nearest respawn by distance without collision check |
| 4a | 0x4083D9 | AI Loop 1 (near ball bonus) | Cave: skip O(N²) loop when count > MAX_BALLS |
| 4b | 0x408548 | AI Loop 2 (find nearest target) | Cave: skip O(N²) loop when count > MAX_BALLS |
| 5 | 0x405190 | Ball_FindClosestRespawnPoint | Cave: throttle to every 60th frame (was 3rd in v4) |

### Key insight

Mesh_FindClosestCollision (0x465D90) is the **most expensive per-ball operation** — it builds a full SpatialTree from level geometry, traverses it for collision, then frees everything. Called 2× per ball in Ball_Update + up to 16× per fallen clone in RespawnPoint search = the main freeze cause.

v6 skips these calls when ball count exceeds MAX_BALLS, and NOPs the respawn-point collision check entirely (the ball just teleports to the nearest point without path-checking — fine for gameplay).

## Usage

1. Rename `bass.dll` → `bass_real.dll` in your Hamsterball folder
2. Copy this `bass.dll` into the game folder
3. Launch the game

The mod waits 5 seconds after DLL load, verifies all byte signatures match, then applies patches.

## Configuration

Change `MAX_BALLS` at top of source and recompile:
- `20` = conservative (very stable)
- `30` = balanced (default)
- `50` = aggressive
- `99` = max (won't crash, may stutter at very high counts)

Change `RESPAWN_THROTTLE` for respawn frequency:
- `60` = once per second at 60fps (default)
- `30` = twice per second
- `120` = every 2 seconds (very stable but slow respawns)

## Build

```
i686-w64-mingw32-gcc -shared -o bass.dll bass_proxy.c \
    -lwinmm -Wl,--enable-stdcall-fixup \
    -O2 -static -static-libgcc -Wl,--add-stdcall-alias
```
