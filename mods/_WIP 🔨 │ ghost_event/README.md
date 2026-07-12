# ghost_event — E:GHOST Custom Event Mod

## What it does

Creates a custom collision event `E:GHOST(filename)` that loads a `.ghost` file (binary format compatible with ghost_saver v22+) and plays it back as a ghost ball — **in any game mode** (Time Trial, Tournament, or Party).

When the ball touches a mesh surface tagged `E:GHOST(PreviousRun)`, the mod:
1. Loads `Ghosts/PreviousRun.ghost` from disk
2. Creates a BestTimeTracker from the ghost data
3. Injects it into `App+0x910` (playback buffer)
4. Creates a ghost ball at `scene+0x361C` if one doesn't exist
5. The game's own `Level_UpdateAndRender` then renders the ghost ball every frame (its Time Trial + party mode checks have been NOP'd out)
6. The mod advances the playback index every frame via an `App_FrameUpdate` epilogue hook

## Usage

1. Place `.ghost` files in a `Ghosts/` directory next to `bass.dll`
   - Files can be created by the ghost_saver mod (Time Trial mode)
   - Binary format: 4-byte magic ("GHSG") + 4-byte version + 4-byte time + 4-byte frameCount + frameCount × 0x28-byte BallSnapshot structs
2. Add `E:GHOST(filename)` event tags to level MESHWORLD files (Section 1 entity names)
3. When the ball touches the tagged surface, the ghost starts playing

## Example

```
E:GHOST(PreviousRun)    → loads Ghosts/PreviousRun.ghost
E:GHOST(Beginner)       → loads Ghosts/Beginner.ghost
E:GHOST(Warm-Up)        → loads Ghosts/Warm-Up.ghost
```

## Installation

1. Rename original `bass.dll` to `bass_real.dll`
2. Copy this mod's `bass.dll` into the game directory
3. Create a `Ghosts/` directory and place `.ghost` files there

## How it works (technical)

### Patches applied

| Address | Original | Patched | Purpose |
|---------|----------|---------|---------|
| 0x40B7F5 | `74 3D` (JZ) | `90 90` (NOP NOP) | Remove Time Trial check in Level_UpdateAndRender |
| 0x40B7FF | `75 33` (JNZ) | `90 90` (NOP NOP) | Remove party mode check in Level_UpdateAndRender |
| 0x40C5D0 | `6A FF 64 A1...` | `E9 ... 90 90 90` | JMP to DCE hook stub |
| 0x46C1F1 | `5E 83 C4 08 C3` | `E9 ...` | JMP to frame epilogue hook stub |

### Playback index advancement

In Time Trial mode, the game's `Scene_UpdateBallsAndState` advances the playback index every frame. In Tournament/Party modes this code is gated on Time Trial mode and never runs, so the index stays at 0 and the ghost never moves.

The mod detects non-TT mode (via `App+0x234 != 0`) and manually advances `BTT_PLAYBACK_IDX` by 1 each frame in the frame epilogue hook, clamping to the last frame. In TT mode this is skipped to avoid double-advancing (which would play the ghost at 2x speed).

### DCE hook

Intercepts `DispatchCollisionEvents` (8-byte entry, `__thiscall`). The raw byte stub saves registers, extracts the event name from the collision entry's MeshBuffer (`pair[1]+0x864`), calls the C handler, then jumps to a trampoline that executes the original 8 bytes and continues.

### Ghost ball creation

Replicates `Board_ctor`'s ghost ball initialization (at 0x419636):
1. `operator_new(0xC60)` — allocate Ball struct
2. `Ball_ctor(ball, scene)` — `__thiscall`, ECX=ball, stack=scene, RET 0x4
3. `ball->vtable[1]()` — `Ball_SetupCollisionRender`
4. Set ghost fields: playerID=-1, gravity=0.5, radius=26.0, maxspeed=1000.0, alpha=0.45
5. Store at `scene+0x361C`

### BTT construction

Allocates a 0x528-byte BestTimeTracker via `operator_new`, sets:
- vtable = 0x4D262C
- AthenaList vtable = 0x4D875C
- count = frame count from ghost file
- list_array = malloc'd array of pointers to each snapshot
- playback_index = 0
- best_time/race_time = finish time from file

### Frame epilogue hook

Runs every frame after `GameUpdate` returns. Advances the playback index by 1 each frame, clamping to the last frame when the recording ends.

## Compatibility

- Works with ghost_saver v22+ binary `.ghost` files
- Also supports legacy format (raw frameCount + finishTime + snapshots without magic header)
- Compatible with Time Trial, Tournament, and Party modes

## Build

```bash
i686-w64-mingw32-gcc -shared -o bass.dll ghost_event.c -I../shared \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
```

## Files

- `ghost_event.c` — Source code
- `bass.dll` — Compiled mod
- `bass_real.dll` — Original BASS audio library (rename from original bass.dll)
