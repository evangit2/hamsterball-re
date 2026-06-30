# Jump Mod (Touch)

## Info
- **File**: `bass.dll` (proxy)
- **Controls**: Tap the screen to jump
- **Android-safe**: Code cave is pure asm, no IAT hooks, no GetTickCount

## What it does
Adds a jump mechanic to the ball! Tap the screen (Winlator maps touch to left-click) and the ball gets an upward impulse.

**How it works:**
1. Background thread polls left mouse button (touch tap) every 16ms
2. On tap: runs a **raycast** straight down from the ball using the game's own `Mesh_FindClosestCollision` function
3. If the raycast hits ground within `radius × 1.45` → ball is grounded → jump allowed
4. If airborne → jump denied (no double jumping)
5. Phase 15 code cave (pure asm): adds upward impulse to `ball+0x174` (Y force)

**Safety gates (jump only works when):**
- Countdown finished (Scene+0x3A4C = 1)
- Race not ended (App+0x5D6 = 0)
- Player not flagged (App+0x5D5 = 0)
- Ball is on the ground (raycast confirmed)

## Winlator Safety
- Phase 15 code cave is **pure FPU asm** (FLD/FADD/FSTP/MOV/JMP) — no C function calls mid-function
- Raycast runs in background thread (safe C context)
- No IAT hooks, no GetTickCount
- All pointer accesses guarded by IsBadReadPtr

## Build
```
i686-w64-mingw32-gcc -shared -o bass.dll jump_touch.c -lwinmm \
  -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## Installation
1. Rename original `bass.dll` to `bass_real.dll`
2. Copy mod `bass.dll` to game folder
3. On Android/Wine: set DLL override to `native` for `bass.dll`
4. Tap screen in-game to jump!
