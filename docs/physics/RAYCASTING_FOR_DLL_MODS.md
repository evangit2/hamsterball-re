# Raycasting for DLL Mods — Reusable Ground Detection Pattern

## Overview

This reference provides a reusable pattern for using the engine's raycast
function (`Mesh_FindClosestCollision` @ `0x00465D90`) from DLL mods to detect
ground contact, wall proximity, or any spatial query against level geometry.

## When to Use Raycasting in a Mod

- **Jump mods** — detect if the ball is on the ground before allowing a jump
- **Wall detection** — check if a wall is ahead/behind/beside the ball
- **Surface probes** — find the height of terrain at a specific XZ coordinate
- **Spawn validation** — verify a position is safe (not inside geometry)
- **Any spatial query** that the engine's own physics system would need

## The Raycast Function

```c
typedef struct { float x, y, z; } Vec3;

/* Mesh_FindClosestCollision — __thiscall
 *   ECX = collision_level (Scene+0x8B0)
 *   Stack: out*, origin(3 floats), direction(3 floats), max_dist(float)
 *   ret 0x20 (32 bytes = 8 DWORDs on stack)
 *
 * Address: 0x00465D90
 * Returns: same pointer as `out` (always — can be ignored)
 */
typedef Vec3* (__thiscall *MeshRaycast_t)(
    void* collision_level,   // ECX = Scene+0x8B0
    Vec3* out,               // output hit point (WRITE target)
    Vec3 origin,             // ray start point (world space)
    Vec3 direction,          // ray direction (will be normalized internally)
    float max_dist           // sphere radius for AABB broad-phase
);

static MeshRaycast_t pfn_raycast = (MeshRaycast_t)0x00465D90;
```

### Critical Typedef Rules

1. **`__thiscall` is mandatory.** Without it, `collision_level` goes on the
   stack instead of ECX, all params shift left by one slot, and the function
   uses garbage for direction — returning wrong results (all directions
   return the same hit point ~26 units below origin).

2. **Vec3 by value = 3 floats on stack.** Ghidra shows these as individual
   `undefined4` stack slots. The C compiler handles this correctly when you
   use the `Vec3` struct typedef — do NOT pass individual floats.

3. **`ret 0x20` (32 bytes).** The function pops 32 bytes (8 DWORDs) from the
   stack on return. The `__thiscall` typedef handles this automatically.

## Accessing Collision Geometry

```c
/* From a Ball pointer: */
void* scene = *(void**)((char*)ball + 0x14);    /* Ball+0x14 = Scene* */
void* cl = *(void**)((char*)scene + 0x8B0);     /* Scene+0x8B0 = CollisionLevel* */

/* From the App global: */
void* app = *(void**)0x005341E0;
void* scene = *(void**)((char*)app + 0x878);     /* App+0x878 = Scene* */
void* cl = *(void**)((char*)scene + 0x8B0);
```

## Key Ball Offsets for Raycasting

| Offset | Type | Field | Notes |
|--------|------|-------|-------|
| `0x014` | `void*` | Scene pointer | → Scene+0x8B0 for CollisionLevel |
| `0x164` | `float` | Position X | Ball+0x59 (int index) |
| `0x168` | `float` | Position Y | Ball+0x5A |
| `0x16C` | `float` | Position Z | Ball+0x5B |
| `0x284` | `float` | Collision radius | Default 26.0, use for max_dist |
| `0xC4C` | `byte` | Fall mode | 1 = dying/respawning, skip raycast |

## Ground Detection Pattern (Downward Raycast)

```c
/* Check if ball is on the ground by casting a downward ray.
 * Returns 1 if ground is within (radius + 2.0) units below the ball. */
int Ball_IsGrounded(void* ball)
{
    char* b = (char*)ball;
    void* scene = *(void**)(b + 0x14);
    if (!scene) return 0;
    void* cl = *(void**)((char*)scene + 0x8B0);
    if (!cl) return 0;

    Vec3 pos = {
        *(float*)(b + 0x164),
        *(float*)(b + 0x168),
        *(float*)(b + 0x16C)
    };
    float radius = *(float*)(b + 0x284);

    /* Downward ray. For standard levels, (0,-1,0) is correct.
     * For tilted-gravity levels, read gravity from CollisionMesh:
     *   void* cm = *(void**)(b + 0x1A4);
     *   Vec3 down = { *(float*)(cm+0xC8C), *(float*)(cm+0xC90), *(float*)(cm+0xC94) };
     */
    Vec3 down = { 0.0f, -1.0f, 0.0f };
    Vec3 out = { 0, 0, 0 };

    /* max_dist = radius + 0.5 (matches engine's own ground probes) */
    pfn_raycast(cl, &out, pos, down, radius + 0.5f);

    /* On hit: out.y ≈ pos.y - radius (floor directly below)
     * On miss: out.y ≈ pos.y - 994 (ray endpoint far away)
     * Check absolute distance */
    float dy = out.y - pos.y;
    if (dy < 0.0f) dy = -dy;
    return (dy <= radius + 2.0f) ? 1 : 0;
}
```

### No-Hit Behavior (IMPORTANT)

When the ray does NOT intersect any geometry, the function does **NOT** return
the origin. It returns the **ray endpoint at ~994 units** along the direction.

- On a downward ray with no floor below: `out.y ≈ pos.y - 994`
- Always check `|out - origin|` distance after the call
- Never assume `out == origin` means "no hit"

### max_dist Semantics

`max_dist` is the **sphere radius for AABB broad-phase**, not a distance limit:

- It expands the query box around the ray
- Spatial tree returns only triangles within this box
- **Use `radius + 0.5f`** (what the engine uses)
- Too large → false hits from off-axis geometry
- Too small → misses nearby geometry

The effective ray length is always ~994 units (direction scaled to 99999,
clamped to 1000 by max_speed, damped to ~994 by friction). `max_dist` does
NOT control how far the ray travels.

## Architecture: Code Cave + Background Thread (Pattern 4)

**NEVER call C functions (including raycast) from a hand-assembled code cave.**
Even with FNSAVE/FNRSTOR, calling a C function from inside a mid-function hook
corrupts the stack/FPU/SEH state and crashes the game.

The correct pattern for mods that need to call game functions:

```
┌─────────────────────────────────────┐
│ Code Cave (inside Ball_Update)      │  Runs per frame (60fps)
│  - Stores ball pointer in volatile  │  Hand-assembled x86, no C calls
│  - Reads volatile flags set by BG   │
│  - Applies mod effects              │
└──────────┬──────────────────────────┘
           │ g_ball_ptr (volatile DWORD)
           ▼
┌─────────────────────────────────────┐
│ Background Thread (Sleep 10ms)     │  Runs ~100x/sec
│  - Reads g_ball_ptr → ball         │  C code, can call game functions
│  - Calls Mesh_FindClosestCollision │
│  - Sets g_on_ground (volatile)     │
└─────────────────────────────────────┘
```

### Template

```c
/* Shared state */
static volatile DWORD g_ball_ptr = 0;    /* set by code cave, read by thread */
static volatile DWORD g_on_ground = 0;   /* set by thread, read by code cave */
static volatile DWORD g_bg_active = 1;

static DWORD WINAPI ground_check_thread(LPVOID param)
{
    (void)param;
    while (g_bg_active) {
        DWORD ball_val = g_ball_ptr;
        if (ball_val) {
            char* ball = (char*)ball_val;
            /* Skip if ball is in death/respawn state */
            if (*(BYTE*)(ball + 0xC4C) == 0) {
                g_on_ground = Ball_IsGrounded(ball);
            } else {
                g_on_ground = 0;
            }
        }
        Sleep(10);
    }
    return 0;
}

/* In DllMain DLL_PROCESS_ATTACH: */
CreateThread(NULL, 0, ground_check_thread, NULL, 0, NULL);

/* In DllMain DLL_PROCESS_DETACH: */
g_bg_active = 0;
WaitForSingleObject(g_bg_thread, 2000);
```

### In the code cave, store the ball pointer:

```asm
/* MOV [g_ball_ptr], ESI  (ESI = ball pointer at Ball_Update hook) */
89 35 <addr_of_g_ball_ptr>
```

### In the code cave, check the ground flag:

```asm
/* MOV EAX, [g_on_ground] */
A1 <addr_of_g_on_ground>
/* TEST EAX, EAX */
85 C0
/* JZ .done (not grounded) */
74 <offset>
```

## Performance Notes

- `Mesh_FindClosestCollision` builds a temp SpatialTree per call — not lightweight
- Engine calls it 2-3 times per ball per tick; 5-10 extra calls per frame is safe
- `max_dist` affects performance: larger = wider AABB = more triangles tested
- Static geometry only — does NOT test against other balls or dynamic objects
- No hit normal returned — only the hit point

## Gravity Direction Per Level

For standard levels, `(0,-1,0)` is the correct downward direction. But some
levels have tilted gravity:

| Level | Gravity Direction | How to detect |
|-------|-------------------|---------------|
| Most levels | `(0, -1, 0)` | Standard |
| Up Race (L6) | `(-1, 0, 0)` | Tilted |
| Odd Race | `(0, 0, 1)` | Flat |

For robust mods, read gravity from the CollisionMesh:

```c
void* cm = *(void**)((char*)ball + 0x1A4);  /* CollisionMesh* (physics body) */
Vec3 down = {
    *(float*)((char*)cm + 0xC8C),  /* grav_dir_x */
    *(float*)((char*)cm + 0xC90),  /* grav_dir_y */
    *(float*)((char*)cm + 0xC94)   /* grav_dir_z */
};
```

## Working Example

See `mods/jump_mod/jump_mod.c` for a complete working implementation of this
pattern — a jump mod that uses raycast ground detection instead of a cooldown
timer.

## Cross-References

- `references/collision-raycast-system.md` — full raycast API, calling convention, max_dist semantics
- `references/ball-ground-detection.md` — why built-in ball flags don't work for ground detection
- `references/jump-ground-check.md` — jump-specific ground check approaches
- `references/bass-proxy-debugging.md` — BASS proxy DLL pattern, code cave pitfalls
- `mods/jump_mod/README.md` — working mod using this pattern
