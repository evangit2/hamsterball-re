# Custom Event Plane Implementation Guide

## Overview

This document explains how to add a **brand-new event plane type** to Hamsterball using a DLL proxy mod. The game's event system is entirely string-matching based — there is no registration table or enum. Adding a new event type means intercepting the string check at runtime.

**Prerequisites:**
- MESHWORLD file with `E:` prefixed geometry (you handle this)
- bass.dll proxy DLL compiled with MinGW (this guide covers it)
- Understanding of the collision dispatch pipeline (see [EVENT_PLANES.md](../physics/EVENT_PLANES.md))

---

## 1. The Dispatch Pipeline (What You're Hooking Into)

When the ball intersects an `E:` named collision mesh, this is the call chain:

```
Ball_FallUpdate (0x408830, vtable[65] at offset 0x104)
  │
  ├─ Collision_TraverseSpatialTree (0x465EF0)
  │   └─ Finds intersecting mesh buffers → populates physics+0x848 list
  │
  └─ For each collision entry in physics+0x848 list:
      └─ board->vtable[0x1D] (offset +0x74)
          │
          ├─ Level_HandleCollision  (0x40DCD0) — race levels (optional override)
          ├─ Arena_HandleCollision  (0x40E6A0) — arenas (optional override)
          └─ DispatchCollisionEvents (0x40C5D0) — shared base, ALWAYS called last
```

**Key insight:** `DispatchCollisionEvents` is the universal chokepoint. Every level-specific handler (`Level_HandleCollision`, `Arena_HandleCollision`, and the 25+ unnamed board-specific handlers) processes its own `E:` events, then falls through to `DispatchCollisionEvents` as a catch-all. Hooking this one function gives you coverage for ALL board types and ALL game modes.

### Calling Convention

```
void __thiscall DispatchCollisionEvents(void *board, int *ball, int *coll_entry);
```

- **ECX** = `board` (the Board/Scene object — type varies by level)
- **[ESP+4]** = `ball` (pointer to Ball struct)
- **[ESP+8]** = `coll_entry` (pointer to 2-element array: `[0]=scene_obj, [1]=mesh_buffer`)

The event name string is at:
```c
char *event_name = *(char **)(coll_entry[1] + 0x864);
```

### Per-Ball State

Each ball has a player index at `ball+0x18` (int, 0–3 for 4 players). This is the same field the game itself uses for per-player scoring in `E:LIMIT` and `N:GOAL`. Use it to index a global array:

```c
static int g_my_flag[4];  // per-player state

void hook(...) {
    int *ball = ...;              // from hook params
    int player_idx = ball[6];     // ball+0x18, int* stride = 0x18
    g_my_flag[player_idx] = 1;
}
```

---

## 2. Hook Architecture

### MinGW `__thiscall` Workaround

MinGW doesn't support `__thiscall` directly. Use `__fastcall` with a dummy EDX parameter:

```c
// __thiscall(this, arg1, arg2) == __fastcall(this, dummy_edx, arg1, arg2)
// Both use ECX for first arg, callee cleans 8 bytes of stack.
typedef void (__fastcall *handler_t)(void *this_, void *edx_dummy, void *ball, void *coll_entry);
```

### Inline Hook (JMP Detour)

The hook overwrites the first 5 bytes of `DispatchCollisionEvents` with a JMP to your handler. A trampoline saves the original 5 bytes + JMP back to original+5:

```
Original:    [orig 5 bytes] [rest of function]
After hook:  [JMP to hook]  [rest of function]
Trampoline:  [orig 5 bytes] [JMP to original+5]
```

Your handler calls the trampoline to execute the original function.

### ASLR Safety

Hamsterball.exe loads at base `0x400000` (no ASLR on this PE), but the hook code computes the real base at runtime:

```c
DWORD base = (DWORD)GetModuleHandleA(NULL);
void *target = (void *)(0x0040C5D0 + (base - 0x00400000));
```

---

## 3. Reading Event Data

### Safe Event Name Access

Always use `IsBadReadPtr` before reading game memory (MinGW doesn't support `__try/__except`):

```c
static const char *get_event_name(void *coll_entry) {
    if (!coll_entry) return "(null)";
    if (IsBadReadPtr(coll_entry, 12)) return "(bad-ptr)";

    int *pair = (int *)coll_entry;
    int mesh_buffer = pair[1];
    if (!mesh_buffer || IsBadReadPtr((void *)mesh_buffer, 0x868))
        return "(bad-mesh)";

    int name_ptr = *(int *)(mesh_buffer + 0x864);
    if (!name_ptr || IsBadReadPtr((void *)name_ptr, 1))
        return "(bad-name)";

    return (const char *)name_ptr;
}
```

### Reading Ball Position

```c
static void get_ball_pos(void *ball, float *x, float *y, float *z) {
    *x = *y = *z = 0.0f;
    if (!ball || IsBadReadPtr(ball, 0x170)) return;
    *x = *(float *)((char *)ball + 0x164);
    *y = *(float *)((char *)ball + 0x168);
    *z = *(float *)((char *)ball + 0x16C);
}
```

### Parsing XML-Style Tag Parameters

If your event name includes parameters (e.g., `E:MYFLAG<DURATION>100</DURATION>`), parse them using the game's own `MWParser_ReadTag` function at `0x0040xxxx` (search Ghidra for `MWParser_ReadTag`). Alternatively, parse manually with `strchr` and `strstr`:

```c
// Parse <TAG>value</TAG> from event name string
static float parse_tag_float(const char *name, const char *tag_name) {
    char open_tag[64], close_tag[64];
    snprintf(open_tag, sizeof(open_tag), "<%s>", tag_name);
    snprintf(close_tag, sizeof(close_tag), "</%s>", tag_name);

    const char *start = strstr(name, open_tag);
    if (!start) return 0.0f;
    start += strlen(open_tag);

    const char *end = strstr(start, close_tag);
    if (!end) return 0.0f;

    char buf[32];
    int len = end - start;
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    memcpy(buf, start, len);
    buf[len] = 0;

    return (float)atof(buf);
}
```

---

## 4. Writing Effects

### Ball Struct Offsets for Event Effects

| Offset | Type | Name | Used By |
|--------|------|------|---------|
| +0x18 | int | player_index | E:LIMIT, N:GOAL (per-player scoring) |
| +0xA7 | float | vert_velocity | E:JUMP (0.025 = upward force) |
| +0xA8 | byte | vert_velocity_on | E:JUMP (= 1, enables vertical velocity) |
| +0x164 | float | pos_x | Ball position X |
| +0x168 | float | pos_y | Ball position Y (vertical, Y-up) |
| +0x16C | float | pos_z | Ball position Z |
| +0x1F7 | int | impact_counter | E:JUMP (10 = bounce cooldown timer) |
| +0x202 | int | freeze_counter | E:JUMP / N:NOCONTROL (10 = freeze input) |
| +0x2D5 | byte | in_water | N:WATER (1 = water physics active) |
| +0xB3 | byte | in_tar | N:TARPIT (1 = tar physics active) |
| +0xB6 | int | zone_timer | N:WATER (10 = effect timer in frames) |
| +0x1DA | byte | velocity_flag | E:LIMIT / N:TARPIT (0 = clear velocity) |

### Calling Game Functions

To trigger existing game effects (sounds, scoring, etc.), call game functions directly:

```c
// Ball_RecordBest(ball, score) — awards points with difficulty modifier
typedef void (__thiscall *Ball_RecordBest_t)(void *ball, long score);
static Ball_RecordBest_t Ball_RecordBest = NULL;

// Sound_Play3D(sound_ptr, x, y, z) — positional audio
typedef void (__cdecl *Sound_Play3D_t)(void *sound, float x, float y, float z);
static Sound_Play3D_t Sound_Play3D = NULL;

// Sound_PlayChannel(channel) — non-positional audio
typedef void (__cdecl *Sound_PlayChannel_t)(int channel);
static Sound_PlayChannel_t Sound_PlayChannel = NULL;
```

**Sound offsets** (from board→App at `board+0x878`→App):
| App Offset | Sound |
|------------|-------|
| +0x460 | Drop-in sound |
| +0x464 | Catapult sound |
| +0x468 | Pop-out sound |
| +0x46C+idx*4 | Pipe bonk sounds (3 variants) |
| +0x484 | Tar sound |
| +0x49C | Jump sound |
| +0x4CC | Popout sound |

### Board/Scene Access

From the `board` parameter (ECX in `__thiscall`):
```c
void *app = *(void **)((char *)board + 0x878);  // App pointer
int difficulty = *(int *)((char *)app + 0x23C); // 0=Pipsqueak, 1=Normal, 2=Frenzied
```

---

## 5. Complete Example: Adding `E:MYFLAG`

This example adds a custom event plane that:
1. Sets a per-player flag when touched
2. Plays the jump sound
3. Awards 100 points
4. Applies a small upward bounce

### Full Source (`custom_event.c`)

```c
/*
 * Hamsterball Custom Event Plane DLL — E:MYFLAG
 *
 * Hooks DispatchCollisionEvents (0x40C5D0) to intercept custom E: event names.
 * Built as a bass.dll proxy (loads automatically with the game).
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll custom_event.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 *
 * Install: Copy bass.dll next to Hamsterball.exe (rename original bass.dll first).
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdlib.h>

/* ── Constants ────────────────────────────────────────────────────────── */

#define GAME_BASE 0x00400000
#define ADDR_DispatchCollisionEvents (GAME_BASE + 0x0000C5D0)
#define TRAMP_SIZE 16

/* ── Types ────────────────────────────────────────────────────────────── */

/* __thiscall workaround: __fastcall with dummy EDX */
typedef void (__fastcall *handler_t)(void *this_, void *edx_dummy,
                                      void *ball, void *coll_entry);

/* Game function typedefs */
typedef void (__thiscall *Ball_RecordBest_t)(void *ball, long score);
typedef void (__cdecl *Sound_Play3D_t)(void *sound, float x, float y, float z);

/* ── Globals ─────────────────────────────────────────────────────────── */

static handler_t g_orig_DispatchCollisionEvents = NULL;
static unsigned char g_tramp[TRAMP_SIZE];

/* Per-player custom flag state (indexed by ball+0x18, max 4 players) */
static int g_my_flag[4] = {0, 0, 0, 0};

/* Game function pointers (resolved at init) */
static Ball_RecordBest_t fn_Ball_RecordBest = NULL;
static Sound_Play3D_t fn_Sound_Play3D = NULL;

/* ── Safe memory helpers ──────────────────────────────────────────────── */

static const char *get_event_name(void *coll_entry) {
    if (!coll_entry) return NULL;
    if (IsBadReadPtr(coll_entry, 12)) return NULL;

    int *pair = (int *)coll_entry;
    int mesh_buffer = pair[1];
    if (!mesh_buffer || IsBadReadPtr((void *)mesh_buffer, 0x868))
        return NULL;

    int name_ptr = *(int *)(mesh_buffer + 0x864);
    if (!name_ptr || IsBadReadPtr((void *)name_ptr, 1))
        return NULL;

    return (const char *)name_ptr;
}

static void get_ball_pos(void *ball, float *x, float *y, float *z) {
    *x = *y = *z = 0.0f;
    if (!ball || IsBadReadPtr(ball, 0x170)) return;
    *x = *(float *)((char *)ball + 0x164);
    *y = *(float *)((char *)ball + 0x168);
    *z = *(float *)((char *)ball + 0x16C);
}

static int get_player_index(void *ball) {
    if (!ball || IsBadReadPtr(ball, 0x20)) return 0;
    return *(int *)((char *)ball + 0x18);
}

/* ── Custom event handler ────────────────────────────────────────────── */

static void handle_my_flag(void *board, void *ball, void *coll_entry) {
    /* Get player index for per-ball state */
    int pidx = get_player_index(ball);
    if (pidx < 0 || pidx > 3) pidx = 0;

    /* Set the custom flag */
    g_my_flag[pidx] = 1;

    /* Play jump sound at ball position */
    if (fn_Sound_Play3D) {
        void *app = *(void **)((char *)board + 0x878);
        if (app && !IsBadReadPtr(app, 0x4A0)) {
            void *jump_sound = *(void **)((char *)app + 0x49C);
            float x, y, z;
            get_ball_pos(ball, &x, &y, &z);
            if (jump_sound)
                fn_Sound_Play3D(jump_sound, x, y, z);
        }
    }

    /* Award 100 points */
    if (fn_Ball_RecordBest)
        fn_Ball_RecordBest(ball, 100);

    /* Small upward bounce (same pattern as E:JUMP) */
    if (ball && !IsBadReadPtr(ball, 0x210)) {
        int *ball_ints = (int *)ball;
        /* Check impact cooldown (ball+0x1F7) */
        if (ball_ints[0x1F7 >> 2] < 1) {
            /* Set vertical velocity */
            *(float *)((char *)ball + 0xA7) = 0.025f;  /* upward force */
            *(unsigned char *)((char *)ball + 0xA8) = 1; /* enable flag */
            ball_ints[0x202 >> 2] = 10;  /* freeze input 10 frames */
            ball_ints[0x1F7 >> 2] = 10;  /* impact cooldown 10 frames */
        }
    }
}

/* ── Hook callback ───────────────────────────────────────────────────── */

void __fastcall hook_DispatchCollisionEvents(void *this_, void *edx_dummy,
                                               void *ball, void *coll_entry) {
    (void)edx_dummy;

    const char *name = get_event_name(coll_entry);

    if (name) {
        /* Check for our custom event */
        if (_stricmp(name, "E:MYFLAG") == 0) {
            handle_my_flag(this_, ball, coll_entry);
            /* Still call original so other events on same geometry fire.
               Return here (skip original) if you want exclusive handling. */
        }

        /* Add more custom events here:
         *
         * if (_strnicmp(name, "E:CUSTOM_SPEED", 14) == 0) {
         *     float power = parse_tag_float(name, "POWER");
         *     handle_speed_pad(this_, ball, coll_entry, power);
         * }
         *
         * if (_stricmp(name, "E:TELEPORT") == 0) {
         *     handle_teleport(this_, ball, coll_entry);
         *     return;  // skip original — teleport replaces all other events
         * }
         */
    }

    /* Call original DispatchCollisionEvents for standard events */
    if (g_orig_DispatchCollisionEvents)
        g_orig_DispatchCollisionEvents(this_, NULL, ball, coll_entry);
}

/* ── Inline hook engine ──────────────────────────────────────────────── */

static int install_hook(void *target, void *hook, unsigned char *trampoline) {
    DWORD oldProtect;
    unsigned char *t = (unsigned char *)target;

    if (!VirtualProtect(t, TRAMP_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;

    /* Save original bytes to trampoline */
    memcpy(trampoline, t, TRAMP_SIZE);

    /* Trampoline: original 5 bytes + JMP back to target+5 */
    trampoline[5] = 0xE9;
    *(unsigned long *)(trampoline + 6) =
        (unsigned long)((char *)target + 5 - (char *)(trampoline + 5) - 5);

    /* Make trampoline executable */
    DWORD tp;
    VirtualProtect(trampoline, TRAMP_SIZE, PAGE_EXECUTE_READWRITE, &tp);

    /* Overwrite target with JMP to hook */
    unsigned long rel = (unsigned long)((char *)hook - (char *)target - 5);
    t[0] = 0xE9;
    *(unsigned long *)(t + 1) = rel;

    FlushInstructionCache(GetCurrentProcess(), target, 5);
    return 1;
}

/* ── Init thread ─────────────────────────────────────────────────────── */

static DWORD WINAPI init_thread(LPVOID lpParam) {
    (void)lpParam;
    Sleep(2000);  /* Wait for game to fully load */

    /* Resolve game function addresses */
    DWORD base = (DWORD)GetModuleHandleA(NULL);
    DWORD offset = base - GAME_BASE;

    fn_Ball_RecordBest = (Ball_RecordBest_t)(0x00402400 + offset);
    fn_Sound_Play3D = (Sound_Play3D_t)(0x0040xxxx + offset);
    /* NOTE: Sound_Play3D address needs Ghidra verification before building.
       Search for "Sound_Play3D" in Ghidra function list. */

    /* Install hook on DispatchCollisionEvents */
    void *target = (void *)(ADDR_DispatchCollisionEvents + offset);
    if (install_hook(target, hook_DispatchCollisionEvents, g_tramp)) {
        g_orig_DispatchCollisionEvents = (handler_t)g_tramp;
    }

    return 0;
}

/* ── DLL Entry (bass.dll proxy) ──────────────────────────────────────── */

/* Forward declarations for BASS exports (see hamsterball-dll-modding skill
   for the full 10-export list) */
BOOL WINAPI BASS_Init(void *a, int b, int c, void *d, void *e) { return TRUE; }
void WINAPI BASS_Free(void) {}
BOOL WINAPI BASS_Start(void) { return TRUE; }
void WINAPI BASS_Stop(void) {}
BOOL WINAPI BASS_SetConfig(int a, int b) { return TRUE; }
/* ... remaining BASS exports forwarded to original if needed ... */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    (void)lpvReserved;
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        CreateThread(NULL, 0, init_thread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        /* Hooks are auto-removed when process exits */
        break;
    }
    return TRUE;
}
```

---

## 6. Build & Test Workflow

### Cross-Compile (Linux → Windows DLL)

```bash
i686-w64-mingw32-gcc -shared -o bass.dll custom_event.c \
    -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
    -Wl,--add-stdcall-alias
```

**MinGW pitfalls (from experience):**
- `fopen()` triggers C4996 error → use `fopen_s()` instead
- `__try/__except` not supported → use `IsBadReadPtr()` before reads
- `dsound.h` may need `#include <mmeapi.h>` before it
- Always use `-static -static-libgcc` to bundle runtime DLLs
- See [skill hamsterball-dll-modding](../../skills/gaming/hamsterball-re/) for full details

### Crash Test (Wine/Xvfb)

```bash
# Copy DLL to game directory
cp bass.dll ~/hamsterball-re/originals/installed/extracted/

# Delete cached files so the game re-reads MESHWORLD
rm -f ~/hamsterball-re/originals/installed/extracted/Levels/*.cached

# Launch game on Xvfb
DISPLAY=:99 LIBGL_ALWAYS_SOFTWARE=1 timeout 35 wine Hamsterball.exe &

# After 35 seconds, check if process is still alive
# (most DLL crashes happen during audio init at startup)
sleep 35
if pgrep -x wine > /dev/null; then
    echo "CRASH TEST PASSED"
else
    echo "CRASH TEST FAILED"
fi
```

**What crash testing validates:**
- Hook address is correct (wrong address = instant crash)
- Calling convention matches (`__thiscall` / `__fastcall`)
- No stack corruption from mismatched `RET N`
- Trampoline preserves original instruction boundaries
- `IsBadReadPtr` guards prevent access violations

**What it does NOT validate:**
- Whether the custom event actually triggers (game renders black on llvmpipe)
- Whether sound plays correctly
- Whether physics effects are correct

Visual/gameplay testing is done on real Windows by the user.

### Testing on Real Windows

1. Backup original `bass.dll` → `bass_real.dll`
2. Copy compiled `bass.dll` next to `Hamsterball.exe`
3. Delete all `*.cached` files in the `Levels/` folder
4. Launch the game
5. Play a level containing your custom `E:MYFLAG` event plane
6. Verify the effect triggers (sound, score, bounce)
7. Use `MessageBoxA` popups in the hook for debugging if needed:

```c
/* Debug: show popup when event fires */
char buf[256];
snprintf(buf, sizeof(buf), "E:MYFLAG hit! player=%d pos=%.1f,%.1f,%.1f",
         pidx, x, y, z);
MessageBoxA(NULL, buf, "CustomEvent", MB_OK);
```

---

## 7. Design Patterns for Common Custom Events

### One-Shot Trigger (disappears after first touch)

```c
static int g_triggered[4] = {0};

void handle_one_shot(void *board, void *ball, void *coll_entry) {
    int pidx = get_player_index(ball);
    if (g_triggered[pidx]) return;  /* already triggered */

    g_triggered[pidx] = 1;
    /* ... apply effect ... */
}
```

### Parameterized Event (parses values from name string)

Use `E:SPEEDPAD<POWER>0.5</POWER>` in the MESHWORLD:

```c
void handle_speed_pad(void *board, void *ball, void *coll_entry,
                      const char *name) {
    /* Parse <POWER> tag */
    const char *start = strstr(name, "<POWER>");
    float power = 0.5f;  /* default */
    if (start) {
        power = (float)atof(start + 7);
    }

    /* Apply directional force based on collision normal */
    int *pair = (int *)coll_entry;
    int mesh = pair[1];
    if (mesh && !IsBadReadPtr((void *)mesh, 0x30)) {
        float nx = *(float *)(mesh + 0x20);
        float ny = *(float *)(mesh + 0x24);
        float nz = *(float *)(mesh + 0x28);
        /* Add force to ball velocity accumulators */
        *(float *)((char *)ball + 0x170) += nx * power;
        *(float *)((char *)ball + 0x174) += ny * power;
        *(float *)((char *)ball + 0x178) += nz * power;
    }
}
```

### Teleporter

```c
static void handle_teleport(void *board, void *ball, void *coll_entry) {
    /* Parse <X>, <Y>, <Z> target from event name */
    const char *name = get_event_name(coll_entry);
    /* ... parse coordinates ... */

    /* Set ball position directly */
    *(float *)((char *)ball + 0x164) = target_x;
    *(float *)((char *)ball + 0x168) = target_y;
    *(float *)((char *)ball + 0x16C) = target_z;

    /* Clear velocity to prevent slingshot */
    *(float *)((char *)ball + 0x170) = 0.0f;
    *(float *)((char *)ball + 0x174) = 0.0f;
    *(float *)((char *)ball + 0x178) = 0.0f;
}
```

### Cooldown-Limited Trigger (rate-limited)

Same pattern as `E:JUMP` — use a counter on the ball:

```c
void handle_cooldown(void *board, void *ball, void *coll_entry) {
    int *ball_ints = (int *)ball;
    /* Check cooldown timer (reuse ball+0x1F7 or another offset) */
    if (ball_ints[0x1F7 >> 2] > 0) return;  /* still cooling down */

    ball_ints[0x1F7 >> 2] = 30;  /* 30-frame cooldown */
    /* ... apply effect ... */
}
```

---

## 8. Reference: All Known Event Types

For reference, here are all existing `E:` and `N:` events the game recognizes. Custom events should use names that don't collide with these:

### `DispatchCollisionEvents` (0x40C5D0) — Universal Events

| Event | Match | Effect |
|-------|------|--------|
| `E:NODIZZY<TIME>N</TIME>` | prefix | Anti-dizzy zone, duration in frames |
| `E:SAFESWITCH` or `E:SAFESWITCH(data)` | prefix | Copy data to ball+0xC2C |
| `E:LIMIT` | exact | Arena finish line tracking |
| `E:BREAK` | exact | Ball bounce callback (vtable[0x20]) |
| `E:JUMP` | exact | Bounce pad: sound + force + 200 score |
| `E:ACTION<ONCE>TRUE</ONCE><SCORE>N</SCORE>` | prefix | One-time score award |
| `E:TRAJECTORY<X>..</X><Y>..</Y><Z>..</Z>` | prefix | Set ball trajectory vector |
| `N:SECRET` | prefix | Mark rotator triggered |
| `N:UNLOCKSECRET` | prefix | Check arena unlock |
| `N:NOCONTROL` | exact | Disable input 10 frames |
| `N:WATER` | exact | Water physics flag + timer |
| `N:TARPIT` | exact | Tar physics flag |
| `N:GOAL` | exact | Race finish sequence |
| `N:MOUSETRAP` | exact | Deflect ball + rotator collision |
| `E:DROPIN` | suffix match | Sound + 200 score (cooldown 50) |
| `E:PIPEBONK` | suffix match | Random sound + 100 score (cooldown 10) |
| `E:POPOUT` | suffix match | Sound + 100 score (cooldown 50) |

### `Level_HandleCollision` (0x40DCD0) — Race-Only Events

| Event | Effect |
|-------|--------|
| `E:CATAPULTBOTTOM` | Launch catapult |
| `E:OPENSESAME` | Open trapdoor |
| `N:TRAPDOOR` | Activate trapdoor |
| `E:BITE` | Set damage=25.0 |
| `E:MACETRIGGER` | Activate all maces |
| `N:MACE` | Ball bounce on mace |

### `Arena_HandleCollision` (0x40E6A0) — Arena-Only Events

| Event | Effect |
|-------|--------|
| `E:CALLHAMMER` | Spawn hammer (tournament only) |
| `E:HAMMERCHASE` | Start hammer chase (tournament only) |
| `E:ALERTSAW1` / `E:ALERTSAW2` | Pre-activate saw blade |
| `E:ACTIVATESAW1` / `E:ACTIVATESAW2` | Full activate saw blade |
| `E:ALERTJUDGES` | Reset all judges |
| `E:SCORE<n>` | Set time on score displays |
| `E:BELL` | Activate bell + 500 bonus time |
| `E:JUMP` | Duplicate of base handler |

### Other Level-Specific Events (in unnamed handlers)

| Event | Found In | Effect |
|-------|----------|--------|
| `E:HEATON` | Neon level handler | Turn heat on |
| `E:HEATOFF` | Neon level handler | Turn heat off |
| `E:NOPEGS` | Toob level handler | Remove pegs |
| `E:PEGS` | Toob level handler | Add pegs |
| `E:TRAPPOP` | Toob level handler | Pop trap |
| `E:LAUNCH` | Tower level handler | Launch ball |

---

## 9. Key Addresses Summary

| Address | Function | Purpose |
|---------|----------|---------|
| `0x0040C5D0` | `DispatchCollisionEvents` | **Hook target** — universal event dispatcher |
| `0x0040DCD0` | `Level_HandleCollision` | Race-specific events (optional secondary hook) |
| `0x0040E6A0` | `Arena_HandleCollision` | Arena-specific events (optional secondary hook) |
| `0x00402400` | `Ball_RecordBest` | Award score to ball |
| `0x00465D90` | `Mesh_FindClosestCollision` | Raycast collision (for custom collision checks) |
| `0x00465260` | `Level_LoadCollision` | How event planes are loaded from MESHWORLD |
| `0x00408830` | `Ball_FallUpdate` | Physics tick that triggers collision dispatch |
| `0x00400000` | Image base | RVA calculations (`addr - 0x400000`) |

### MeshBuffer Struct (0x874 bytes)

| Offset | Type | Field |
|--------|------|-------|
| +0x85D | byte | interactive (1 for `N:` and `E:` prefixes) |
| +0x863 | byte | no_render (1 for `E:` prefix only) |
| +0x864 | char* | Event name string pointer |

### Collision Entry (passed as `coll_entry`)

| Offset | Type | Field |
|--------|------|-------|
| +0x00 | void* | Scene object pointer (Level/Stands) |
| +0x04 | void* | Mesh buffer pointer (has +0x864 = name string) |

---

## 10. Troubleshooting

### Hook doesn't fire

- Verify the DLL is loaded (add `MessageBoxA(NULL, "loaded", "", MB_OK)` in `DllMain`)
- Check that `Sleep(2000)` delay is long enough for game init
- Verify hook address: decompile `DispatchCollisionEvents` at `0x40C5D0` in Ghidra — the first 5 bytes must be a complete instruction (no mid-instruction split)
- If using bass.dll proxy: ensure the original `bass.dll` is renamed to `bass_real.dll`, not deleted

### Event name is `(null)` or `(bad-ptr)`

- The `coll_entry` pointer may be invalid — check `IsBadReadPtr` guards
- The mesh buffer at `coll_entry[1]` may not have an event name (it's regular geometry, not an event plane)
- Only meshes with `E:` or `N:` prefix names have the string at +0x864 populated

### Game crashes on event trigger

- Most common: calling convention mismatch. Verify `DispatchCollisionEvents` uses `__thiscall` (ECX = this, callee cleans 8 bytes). Your hook must match.
- Stack corruption: ensure your hook function doesn't push extra args or use wrong `RET N`
- Writing to invalid ball offset: always `IsBadReadPtr` before writing
- Calling game functions with wrong calling convention: `Ball_RecordBest` is `__thiscall`, `Sound_Play3D` is `__cdecl`

### Event fires but no effect

- The event name string might not match exactly. Use `_stricmp` for case-insensitive exact match, or `_strnicmp` for prefix match
- The event might be firing on the wrong ball (AI ball instead of player). Check `ball+0x18` to verify player index
- Sound pointers might be null — verify `app+0x49C` is non-zero before calling `Sound_Play3D`

### Visual verification on Wine/llvmpipe

- Game renders black on llvmpipe — visual testing is not possible on Linux
- Use `MessageBoxA` popups or `OutputDebugStringA` logging for verification
- Full visual/gameplay testing requires real Windows hardware
