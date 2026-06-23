# Hamsterball Collision Hook

## What This Does

Hooks three collision dispatch functions in Hamsterball.exe to log all collision events:

1. **DispatchCollisionEvents** (0x0040C5D0) — shared base handler, processes ALL event types
2. **Level_HandleCollision** (0x0040DCD0) — race level events (catapults, trapdoors, maces)
3. **Arena_HandleCollision** (0x0040E6A0) — arena events (hammers, saws, judges, bells)

## Files

| File | Description |
|------|-------------|
| `collision_hook.dll` | The hook DLL — inject into Hamsterball.exe |
| `injector.exe` | Standalone DLL injector (finds Hamsterball.exe automatically) |
| `collision_hook.cfg` | Config file — enable/disable hooks, set filters |
| `collision_log.csv` | Output log (created automatically when game runs) |

## Quick Start

1. Copy all files into your Hamsterball game directory (next to `Hamsterball.exe`)
2. Launch `Hamsterball.exe` normally
3. Run `injector.exe` (double-click or command line)
4. Play the game — collision events are logged to `collision_log.csv`
5. Check the CSV in any spreadsheet editor or text editor

## Config File (`collision_hook.cfg`)

```ini
# Enable/disable individual hooks (1=on, 0=off)
hook_DispatchCollisionEvents=1
hook_Level=1
hook_Arena=1

# Log options
log_event_names=1
log_ball_pos=1
log_timestamps=1

# Filter: only log events matching this substring (empty = log all)
# Examples: filter=E:JUMP  or  filter=N:GOAL  or  filter= (empty = all)
filter=
```

## CSV Output Format

```
timestamp_ms,handler,scene_ptr,ball_ptr,collobj_ptr,event_name,x,y,z
```

- `timestamp_ms` — milliseconds since Windows boot (GetTickCount)
- `handler` — which function was called (DispatchCollisionEvents/Level_HandleCollision/Arena_HandleCollision)
- `scene_ptr` — pointer to the Scene/BoardLevel object (ECX/this)
- `ball_ptr` — pointer to the Ball object
- `collobj_ptr` — pointer to the collision pair array
- `event_name` — the event string from the collision object (e.g. "E:JUMP", "N:GOAL")
- `x,y,z` — ball position at time of collision

## How It Works

The DLL installs inline hooks (jmp detours) at the entry point of each target
function. When the game calls a collision handler, our hook intercepts the call,
logs the event details, then forwards to the original function via a trampoline.

```
Hamsterball.exe calls DispatchCollisionEvents(this, ball, collObj)
  → jmp to hook_DispatchCollisionEvents
    → log event to CSV
    → call original via trampoline
      → execute relocated original bytes
      → jmp back to DispatchCollisionEvents+5
```

## Technical Details

- **Target addresses** (image base 0x400000):
  - DispatchCollisionEvents: VA 0x0040C5D0
  - Level_HandleCollision: VA 0x0040DCD0
  - Arena_HandleCollision: VA 0x0040E6A0
- **Calling convention**: `__thiscall` (ECX = this/Scene pointer, stack = ball, collObj)
- **ASLR**: The DLL computes the actual address by adding the module base offset
- **Trampoline**: 5-byte original prologue + jmp back to original+5

## Safety

- Hooks are **read-only** — they only log events, never modify game behavior
- Hooks are cleanly removed on DLL unload (DLL_PROCESS_DETACH)
- All pointer reads use SEH (__try/__except) to prevent crashes from bad pointers
- The injector uses standard CreateRemoteThread + LoadLibrary technique

## Building from Source

```bash
# DLL
i686-w64-mingw32-gcc -shared -o collision_hook.dll collision_hook.c \
    -Wl,--enable-stdcall-fixup -Wl,--out-implib,collision_hook.lib

# Injector
i686-w64-mingw32-gcc -o injector.exe injector.c
```

## Limitations

- Does not modify game behavior (logging only). For actual modding, extend
  `my_DispatchCollisionEvents_handler` etc. to modify parameters or return values.
- Inline hook assumes first 5 bytes of target function are complete instructions
  (no instruction boundary split at byte 5). Verified for Hamsterball.exe.
- Requires Windows (32-bit process). Won't work under Wine (needs SEH + inline hooks).
