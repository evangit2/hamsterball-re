/*
 * WaterMod_MinGW.cpp — Water Physics Mod for HB+ v2.1 (MinGW build)
 *
 * MinGW-safe variant of WaterMod.cpp (the MSVC class-based version).
 * Uses the manual 17-entry vtable + nocrt + hbplus_api.h so it compiles
 * with i686-w64-mingw32-g++ and loads correctly under HB+.
 *
 * Port of hamsterball_water_mod v7.9 (bass.dll proxy) to HB+ API.
 *
 * v7.9 fixes (parity with bass v7.9):
 *   F1 — Hook 4 now patches the PLAYER ball vtable slot 8 @ 0x4CF334
 *     (base 0x4CF314, Ball_SplitDeath). Previously 0x4CF3A0 (BADBALL
 *     vtable) was hooked, so the player was never protected.
 *   F2 — g_states table: dead-slot recycling in get_ball_state +
 *     lookup-only peek_ball_state for Hook 4 (no more table exhaustion).
 *   F3 — prev_submersion=1.0 on water entry (no more entry-frame
 *     double-damp of Y velocity).
 *
 * Features (parity with bass v7.8):
 * - E:WATER collision event triggers water entry (velocity damping + surface capture)
 * - E:WATEREXIT turns water OFF entirely with NO grace period
 * - E:WATERFLOW(N) = E:WATER subset + running-water current (force accumulators),
 *   direction 1-8 clockwise from N, switches immediately within same body of water
 * - Per-frame drag, horizontal drag, and buoyancy while submerged
 * - Dizzy immunity while submerged (clears bounce counter + sets immunity timer)
 * - Fall death suppression during water + 120-frame grace period after exit
 * - Type 5 mesh-penetration suppression while submerged/in grace
 *
 * Author: RodentRacer / Hamsterbot
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

#include <windows.h>

#define malloc nc_malloc
#define free nc_free
#define memcpy nc_memcpy
#define memset nc_memset
#define strlen nc_strlen
#define strcmp nc_strcmp
#define strncmp nc_strncmp
#define memcmp nc_memcmp
#define strncpy nc_strncpy
#define snprintf nc_snprintf

/* Minimal strstr — returns first occurrence of needle in haystack, or NULL. */
static const char* nc_strstr(const char* haystack, const char* needle) {
    if (!haystack || !needle) return NULL;
    if (*needle == '\0') return haystack;
    for (; *haystack; haystack++) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (*n == '\0') return haystack;
    }
    return NULL;
}
#define strstr nc_strstr

/* ═══════════════════════════════════════════════════════════════════════════
 * Constants
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Ball struct offsets */
#define BALL_POS_X              0x164
#define BALL_POS_Y              0x168
#define BALL_POS_Z              0x16C
#define BALL_PHYS_PTR           0x1A4
#define BALL_RADIUS             0x284
#define BALL_FALLING_FLAG        0x2E9   /* BYTE: set by type 5 collision, E:LIMIT */
#define BALL_BOUNCE_COUNT       0x2EC   /* INT: dizzy bounce counter */
#define BALL_DIZZY_IMMUNITY     0x2F4   /* INT: dizzy immunity timer in frames */

/* Physics struct offsets */
#define PHYS_VEL_X              0xCA4
#define PHYS_VEL_Y              0xCA8
#define PHYS_VEL_Z              0xCAC

/* E:WATERFLOW(N) — direction suffix parsed from the plane name.
 *  1=N(-Z) 2=NE(+X,-Z) 3=E(+X) 4=SE(+X,+Z)
 *  5=S(+Z) 6=SW(-X,+Z) 7=W(-X) 8=NW(-X,-Z); 0 = calm, no flow. */
#define WATERFLOW_PREFIX        "E:WATERFLOW"
#define WATERFLOW_PREFIX_LEN    11      /* strlen("E:WATERFLOW") — points at '(' */
#define FLOW_NONE               0
#define FLOW_FORCE_NORMALIZE    0.7071067811865476f /* 1/sqrt(2), diagonals */
#define DEFAULT_FLOW_FORCE      0.18f   /* per-frame force into accumulators */

/* Ball force accumulators (physics inputs — do NOT write position directly) */
#define BALL_FORCE_X            0x170
#define BALL_FORCE_Y            0x174
#define BALL_FORCE_Z            0x178

#define GRACE_PERIOD_FRAMES     120     /* ~5 seconds at 25fps */
#define MAX_BALLS               32

/* Hook 3: Type 5 collision suppressor — code cave at 0x407377 in Ball_Update */
#define ADDR_TYPE5_JNZ          0x00407377
#define TYPE5_SKIP_TARGET       0x0040743D
#define TYPE5_NEXT_INSTR        0x0040737D
#define TYPE5_PATCH_SIZE        6

/* Hook 4: Ball vtable[8] — Ball_FallDeath suppression.
 * v7.9: Hook the PLAYER ball vtable (base 0x4CF314, slot 8 @ 0x4CF334 =
 * Ball_SplitDeath 0x409050). The player's death dispatch (CALL vtable[8]
 * at 0x406C76/0x40721F) reads THIS slot. The old value 0x4CF3A0 was the
 * BADBALL vtable (slot 8 @ 0x4CF3C0 = Ball_FallDeath 0x409480) — that
 * protected BadBalls from fall death but never the player. */
#define ADDR_BALL_VTABLE        0x004CF314
#define VTABLE_SLOT_FALLDEATH   8

/* ═══════════════════════════════════════════════════════════════════════════
 * Config (static globals, updated via HB+ sliders)
 * ═══════════════════════════════════════════════════════════════════════════ */

static float g_entry_damping     = 0.90f;
static float g_drag              = 0.02f;
static float g_horizontal_drag   = 0.04f;
static float g_buoyancy_strength = 1.0f;
static float g_flow_force        = DEFAULT_FLOW_FORCE;
static bool  g_enabled           = true;

static IModAPI* g_api = NULL;
static void* g_modObj = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-ball water state (static — accessible from code cave + callbacks)
 * ═══════════════════════════════════════════════════════════════════════════ */

struct WaterState {
    DWORD ball;              /* ball pointer (key); 0 = unused */
    int   in_water;          /* currently in water? */
    float water_surface_y;   /* ball's Y at moment of contact */
    int   grace_frames;      /* death suppression frames after leaving water */
    float prev_submersion;   /* last frame's submersion (surface crossing) */
    int   flow_dir;          /* E:WATERFLOW(N): 1-8, 0 = calm */
};

static WaterState g_states[MAX_BALLS];

static WaterState* get_ball_state(DWORD ball) {
    int free_idx = -1;
    for (int i = 0; i < MAX_BALLS; i++) {
        if (g_states[i].ball == ball) return &g_states[i];
        if (free_idx == -1 && g_states[i].ball == 0) free_idx = i;
    }
    /* v7.9: recycle dead slots before allocating. A slot whose ball is
     * neither in water nor in grace is stale (ball fell to death, was
     * respawned, or left the level) — its pointer key is dead. Reuse it
     * instead of letting the table fill up with corpses. */
    if (free_idx == -1) {
        for (int i = 0; i < MAX_BALLS; i++) {
            if (g_states[i].in_water == 0 && g_states[i].grace_frames == 0) {
                free_idx = i;
                break;
            }
        }
    }
    if (free_idx >= 0) {
        memset(&g_states[free_idx], 0, sizeof(WaterState));
        g_states[free_idx].ball = ball;
        return &g_states[free_idx];
    }
    return NULL;
}

/* Lookup-only getter — never allocates a slot. Used by Hook 4
 * (fall-death suppression) where the ball may have no water state at
 * all; allocating a slot there would leak table entries for every ball
 * that ever falls to death outside water. */
static WaterState* peek_ball_state(DWORD ball) {
    if (!ball) return NULL;
    for (int i = 0; i < MAX_BALLS; i++) {
        if (g_states[i].ball == ball) return &g_states[i];
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Water physics (same logic as bass.dll v7)
 * ═══════════════════════════════════════════════════════════════════════════ */

static float compute_submersion(float ball_y, float radius, float surface_y) {
    if (radius <= 0.0f) return 0.0f;
    float sub = (surface_y - ball_y + radius) / (2.0f * radius);
    if (sub < 0.0f) sub = 0.0f;
    if (sub > 1.0f) sub = 1.0f;
    return sub;
}

/* Parse the direction number from an E:WATERFLOW(N) name.
 * Returns the flow direction flag (1-8), or FLOW_NONE (0)
 * if there is no flow / the direction is unknown. A plain E:WATER name
 * (no "FLOW" suffix) naturally returns FLOW_NONE = calm water.
 *   format: E:WATERFLOW(N)  where N is 1-8
 *   1=N(-Z) 2=NE(+X,-Z) 3=E(+X) 4=SE(+X,+Z)
 *   5=S(+Z) 6=SW(-X,+Z) 7=W(-X) 8=NW(-X,-Z) */
static int parse_flow_direction(const char* name) {
    if (!name || !strstr(name, "FLOW")) return FLOW_NONE;
    const char* d = name + WATERFLOW_PREFIX_LEN;
    if (*d != '(') return FLOW_NONE;
    d++;
    if (*d >= '1' && *d <= '8' && d[1] == ')')
        return *d - '0';
    return FLOW_NONE;
}

/* Trigger: called from onEventPlaneCollide when ball touches E:WATER.
 * Does the E:WATER entry trigger: set flag, damp velocity, capture Y.
 * If the plane is E:WATERFLOW(N), additionally sets the running-water
 * direction flag (flow_dir 1-8). Plain E:WATER has flow_dir = 0 (calm). */
static void trigger_water_contact(DWORD ball, const char* name) {
    if (!ball || IsBadReadPtr((void*)ball, 0x300)) return;

    float ball_y = *(float*)(ball + BALL_POS_Y);
    float radius = *(float*)(ball + BALL_RADIUS);
    if (radius <= 0.0f || radius > 1000.0f) return;

    DWORD phys = *(DWORD*)(ball + BALL_PHYS_PTR);
    if (!phys || IsBadReadPtr((void*)phys, 0xCB0)) return;

    WaterState* st = get_ball_state(ball);
    if (!st) return;

    /* If already in water, only the flow direction can change.
     * Touching a different E:WATERFLOW(N) plane inside the same body of
     * water switches the current direction immediately (no re-damp, no
     * re-capture — the ball never left the water). Touching plain
     * E:WATER (calm) here clears the flow (parse returns FLOW_NONE).
     * E:WATEREXIT is handled separately and never reaches this function. */
    if (st->in_water) {
        int new_dir = parse_flow_direction(name);
        if (new_dir != st->flow_dir) st->flow_dir = new_dir;
        return;
    }

    /* Set in_water flag */
    st->in_water = 1;
    st->flow_dir = parse_flow_direction(name);

    /* Clear falling flag (set during long fall before reaching water) */
    *(BYTE*)(ball + BALL_FALLING_FLAG) = 0;

    /* Reset bounce counter (same as E:NODIZZY) */
    *(int*)(ball + BALL_BOUNCE_COUNT) = 0;

    /* Damp velocity on entry */
    float damp = g_entry_damping;
    *(float*)(phys + PHYS_VEL_X) *= damp;
    *(float*)(phys + PHYS_VEL_Y) *= damp;
    *(float*)(phys + PHYS_VEL_Z) *= damp;

    /* Capture water surface Y */
    st->water_surface_y = ball_y;

    /* v7.9 (F3): Mark the ball as already-submerged this entry frame.
     * Without this, prev_submersion defaults to 0.0, so the surface
     * crossing check in apply_water_physics fires on the SAME frame
     * (submersion >= 0.5) and damps Y velocity a second time —
     * effective damping becomes entry_damping^2 (e.g. 0.9*0.9 = 0.81)
     * on the entry frame. Setting prev_submersion = 1.0 makes the
     * crossing check see "already submerged" and skip the re-damp;
     * the entry damping above is the ONLY damping applied on entry. */
    st->prev_submersion = 1.0f;
}

/* Trigger: called when the ball touches an E:WATEREXIT plane.
 * Turns OFF the water flag entirely and immediately:
 *   - in_water = 0 (stops all water physics instantly)
 *   - water_surface_y / prev_submersion reset
 *   - grace_frames = 0 (NO grace period — the exit is final) */
static void trigger_water_exit(DWORD ball) {
    if (!ball || IsBadReadPtr((void*)ball, 0x300)) return;

    WaterState* st = get_ball_state(ball);
    if (!st) return;

    /* Idempotent: if already out of water, nothing to do. */
    if (!st->in_water) return;

    st->in_water = 0;
    st->water_surface_y = 0.0f;
    st->prev_submersion = 0.0f;
    st->grace_frames = 0;
    st->flow_dir = FLOW_NONE;
}

/* Per-frame physics: called from onBallUpdate */
static void apply_water_physics(DWORD ball) {
    if (!ball || IsBadReadPtr((void*)ball, 0x300)) return;

    WaterState* st = get_ball_state(ball);
    if (!st) return;

    /* Decrement grace timer every frame */
    if (st->grace_frames > 0) st->grace_frames--;

    /* Clear 0x2E9 and grant dizzy immunity while submerged */
    if (st->in_water) {
        *(BYTE*)(ball + BALL_FALLING_FLAG) = 0;
        *(int*)(ball + BALL_BOUNCE_COUNT) = 0;
        int cur = *(int*)(ball + BALL_DIZZY_IMMUNITY);
        if (cur < GRACE_PERIOD_FRAMES)
            *(int*)(ball + BALL_DIZZY_IMMUNITY) = GRACE_PERIOD_FRAMES;
    }

    if (!st->in_water) return;

    float ball_y = *(float*)(ball + BALL_POS_Y);
    float radius = *(float*)(ball + BALL_RADIUS);
    if (radius <= 0.0f || radius > 1000.0f) return;

    /* Exit condition: ball clearly above surface */
    if (ball_y - radius > st->water_surface_y + radius * 0.5f) {
        st->in_water = 0;
        st->water_surface_y = 0.0f;
        st->prev_submersion = 0.0f;
        st->grace_frames = GRACE_PERIOD_FRAMES;
        return;
    }

    DWORD phys = *(DWORD*)(ball + BALL_PHYS_PTR);
    if (!phys || IsBadReadPtr((void*)phys, 0xCB0)) return;

    float submersion = compute_submersion(ball_y, radius, st->water_surface_y);

    /* Surface crossing detection: apply entry damping on re-entry */
    if (st->prev_submersion < 0.5f && submersion >= 0.5f) {
        *(float*)(phys + PHYS_VEL_Y) *= g_entry_damping;
    }
    st->prev_submersion = submersion;

    /* Apply drag */
    float vscale = 1.0f - g_drag;
    float hscale = 1.0f - (g_drag + g_horizontal_drag);
    if (vscale < 0.0f) vscale = 0.0f;
    if (hscale < 0.0f) hscale = 0.0f;

    *(float*)(phys + PHYS_VEL_X) *= hscale;
    *(float*)(phys + PHYS_VEL_Z) *= hscale;
    *(float*)(phys + PHYS_VEL_Y) *= vscale;

    /* Buoyancy = strength * submersion (0-1) */
    float buoyancy = g_buoyancy_strength * submersion;
    *(float*)(phys + PHYS_VEL_Y) += buoyancy;

    /* Running-water current (E:WATERFLOW).
     * Adds a constant per-frame force (flow_force) into the ball's force
     * accumulators (ball+0x170/174/178) in the flow direction. The physics
     * engine consumes these accumulators with proper collision response,
     * so the current pushes/carries the ball without teleporting it, and
     * is naturally capped by drag/friction exactly like other forces.
     * flow_dir: 1=N 2=NE 3=E 4=SE 5=S 6=SW 7=W 8=NW; 0 = calm, no force. */
    if (st->flow_dir != FLOW_NONE) {
        float* fx = (float*)((DWORD)ball + BALL_FORCE_X);
        float* fz = (float*)((DWORD)ball + BALL_FORCE_Z);
        float f = g_flow_force;
        switch (st->flow_dir) {
            case 1:  *fz -= f; break;                                              /* N  */
            case 2:  *fx += f * FLOW_FORCE_NORMALIZE; *fz -= f * FLOW_FORCE_NORMALIZE; break; /* NE */
            case 3:  *fx += f; break;                                              /* E  */
            case 4:  *fx += f * FLOW_FORCE_NORMALIZE; *fz += f * FLOW_FORCE_NORMALIZE; break; /* SE */
            case 5:  *fz += f; break;                                              /* S  */
            case 6:  *fx -= f * FLOW_FORCE_NORMALIZE; *fz += f * FLOW_FORCE_NORMALIZE; break; /* SW */
            case 7:  *fx -= f; break;                                              /* W  */
            case 8:  *fx -= f * FLOW_FORCE_NORMALIZE; *fz -= f * FLOW_FORCE_NORMALIZE; break; /* NW */
            default: break;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook 3: Type 5 collision suppressor (code cave)
 * ═══════════════════════════════════════════════════════════════════════════ */

static int __cdecl is_ball_in_water(DWORD ball) {
    if (!ball || IsBadReadPtr((void*)ball, 0x300)) return 0;
    WaterState* st = get_ball_state(ball);
    return (st && (st->in_water || st->grace_frames > 0)) ? 1 : 0;
}

static int (__cdecl *g_is_in_water_ptr)(DWORD) = NULL;
static BYTE* g_type5_cave = NULL;

static void install_type5_hook() {
    BYTE* hook_addr = (BYTE*)ADDR_TYPE5_JNZ;

    /* Verify expected bytes: 0F 85 C0 00 00 00 (JNZ rel32) */
    BYTE expected[] = { 0x0F, 0x85, 0xC0, 0x00, 0x00, 0x00 };
    if (memcmp(hook_addr, expected, TYPE5_PATCH_SIZE) != 0) return;

    g_type5_cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                         PAGE_EXECUTE_READWRITE);
    if (!g_type5_cave) return;

    g_is_in_water_ptr = is_ball_in_water;

    int p = 0;

    /* PUSHFD — save original flags (from penetration check) */
    g_type5_cave[p++] = 0x9C;
    /* PUSH ESI — ball pointer, becomes __cdecl argument */
    g_type5_cave[p++] = 0x56;
    /* CALL [g_is_in_water_ptr] */
    g_type5_cave[p++] = 0xFF; g_type5_cave[p++] = 0x15;
    *(DWORD*)(g_type5_cave + p) = (DWORD)&g_is_in_water_ptr; p += 4;
    /* ADD ESP, 4 — clean up __cdecl arg */
    g_type5_cave[p++] = 0x83; g_type5_cave[p++] = 0xC4; g_type5_cave[p++] = 0x04;
    /* TEST EAX, EAX — check return value */
    g_type5_cave[p++] = 0x85; g_type5_cave[p++] = 0xC0;
    /* JNZ to in-water cleanup (placeholder, patched later) */
    g_type5_cave[p++] = 0x0F; g_type5_cave[p++] = 0x85;
    int jnz_inwater_offset = p;
    p += 4;

    /* --- Not in water path: --- */
    /* POPFD — restore original flags */
    g_type5_cave[p++] = 0x9D;
    /* JNZ 0x40743D — reproduce original conditional jump */
    g_type5_cave[p++] = 0x0F; g_type5_cave[p++] = 0x85;
    {
        DWORD src = (DWORD)(g_type5_cave + p + 4);
        *(DWORD*)(g_type5_cave + p) = (DWORD)TYPE5_SKIP_TARGET - src;
    }
    p += 4;
    /* Fall through: penetration > 1.0 and not in water → continue to 0x40737D */
    g_type5_cave[p++] = 0xE9;
    *(DWORD*)(g_type5_cave + p) = (DWORD)TYPE5_NEXT_INSTR - (DWORD)(g_type5_cave + p + 4);
    p += 4;

    /* --- In water cleanup path: --- */
    int inwater_cleanup_addr = p;
    /* ADD ESP, 4 — remove saved EFLAGS */
    g_type5_cave[p++] = 0x83; g_type5_cave[p++] = 0xC4; g_type5_cave[p++] = 0x04;
    /* JMP to TYPE5_SKIP_TARGET — skip entire type 5 death block */
    g_type5_cave[p++] = 0xE9;
    {
        DWORD src = (DWORD)(g_type5_cave + p + 4);
        *(DWORD*)(g_type5_cave + p) = (DWORD)TYPE5_SKIP_TARGET - src;
    }
    p += 4;

    /* Patch the in-water JNZ to target the cleanup path */
    {
        DWORD src = (DWORD)(g_type5_cave + jnz_inwater_offset + 4);
        DWORD dst = (DWORD)(g_type5_cave + inwater_cleanup_addr);
        *(DWORD*)(g_type5_cave + jnz_inwater_offset) = dst - src;
    }

    /* Patch hook site: 5-byte JMP + 1 NOP */
    DWORD old_protect;
    VirtualProtect(hook_addr, TYPE5_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &old_protect);
    *(DWORD*)(hook_addr + 1) = (DWORD)g_type5_cave - (DWORD)hook_addr - 5;
    hook_addr[0] = 0xE9;
    hook_addr[5] = 0x90;
    VirtualProtect(hook_addr, TYPE5_PATCH_SIZE, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, TYPE5_PATCH_SIZE);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook 4: Ball vtable[8] — Ball_FallDeath suppression
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef void (__thiscall *ball_falldeath_t)(void *ball);
static ball_falldeath_t orig_Ball_FallDeath = NULL;

static void __thiscall Hook_Ball_FallDeath(void *ball) {
    if (ball) {
        /* Lookup only — do NOT allocate a state slot here (v7.9).
         * Every ball that falls to death outside water would otherwise
         * leak a table entry and eventually exhaust the 32-slot table,
         * silently killing water physics for the whole session. */
        WaterState* st = peek_ball_state((DWORD)ball);
        if (st && (st->in_water || st->grace_frames > 0)) {
            return;  /* suppress death */
        }
    }
    if (orig_Ball_FallDeath) orig_Ball_FallDeath(ball);
}

static void install_falldeath_hook() {
    void** slot = (void**)(ADDR_BALL_VTABLE + VTABLE_SLOT_FALLDEATH * sizeof(void*));
    if (IsBadReadPtr(slot, sizeof(void*))) return;

    DWORD old_protect;
    VirtualProtect(slot, sizeof(void*), PAGE_EXECUTE_READWRITE, &old_protect);
    orig_Ball_FallDeath = (ball_falldeath_t)*slot;
    *slot = (void*)Hook_Ball_FallDeath;
    VirtualProtect(slot, sizeof(void*), old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), slot, sizeof(void*));
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HB+ vtable callbacks (manual vtables — MinGW-safe)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "Water Mod"; }
static const char* __thiscall get_author(void*) { return "RodentRacer"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    g_api = api;
    g_modObj = thisptr;

    CustomButton btn("WATER_MOD", "Water Physics");
    btn.defaultState = true;
    HBAPI(api).CreateToggleButton(btn, thisptr);

    CustomSlider s1("WATER_ENTRY_DAMP", "Entry Damping", 0.90f);
    s1.lowerBound = 0.0f; s1.upperBound = 1.0f; s1.stepSize = 0.05f; s1.decimalPlaces = 2;
    HBAPI(api).CreateSlider(s1, thisptr);

    CustomSlider s2("WATER_DRAG", "Drag", 0.02f);
    s2.lowerBound = 0.0f; s2.upperBound = 0.1f; s2.stepSize = 0.005f; s2.decimalPlaces = 3;
    HBAPI(api).CreateSlider(s2, thisptr);

    CustomSlider s3("WATER_HDRAG", "Horizontal Drag", 0.04f);
    s3.lowerBound = 0.0f; s3.upperBound = 0.1f; s3.stepSize = 0.005f; s3.decimalPlaces = 3;
    HBAPI(api).CreateSlider(s3, thisptr);

    CustomSlider s4("WATER_BUOY", "Buoyancy", 1.0f);
    s4.lowerBound = 0.0f; s4.upperBound = 2.0f; s4.stepSize = 0.1f; s4.decimalPlaces = 2;
    HBAPI(api).CreateSlider(s4, thisptr);

    CustomSlider s5("WATER_FLOW", "Current Strength", DEFAULT_FLOW_FORCE);
    s5.lowerBound = 0.0f; s5.upperBound = 1.0f; s5.stepSize = 0.01f; s5.decimalPlaces = 2;
    HBAPI(api).CreateSlider(s5, thisptr);
}

static void __thiscall button_toggle(void*, const char* id, bool state) {
    if (strcmp(id, "WATER_MOD") == 0) g_enabled = state;
}

static void __thiscall slider_change(void*, const char* id, float value) {
    if      (strcmp(id, "WATER_ENTRY_DAMP") == 0) g_entry_damping = value;
    else if (strcmp(id, "WATER_DRAG") == 0)       g_drag = value;
    else if (strcmp(id, "WATER_HDRAG") == 0)       g_horizontal_drag = value;
    else if (strcmp(id, "WATER_BUOY") == 0)        g_buoyancy_strength = value;
    else if (strcmp(id, "WATER_FLOW") == 0)        g_flow_force = value;
}

static bool g_hooksInstalled = false;

static void __thiscall game_update(void*) {
    /* Install raw memory hooks once (deferred to first game update) */
    if (!g_hooksInstalled) {
        install_type5_hook();
        install_falldeath_hook();
        g_hooksInstalled = true;
    }
}

static void __thiscall event_collide(void*, void* ball, const char* eventPlaneID) {
    if (!g_enabled || !ball || !eventPlaneID) return;

    /* E:WATEREXIT is checked FIRST — it starts with "E:WATER" so the
     * prefix match below would swallow it. E:WATERFLOW(N) intentionally
     * falls through to trigger_water_contact, which parses the direction
     * from the name. */
    if (strncmp(eventPlaneID, "E:WATEREXIT", 11) == 0) {
        trigger_water_exit((DWORD)ball);
    } else if (strncmp(eventPlaneID, "E:WATER", 7) == 0) {
        trigger_water_contact((DWORD)ball, eventPlaneID);
    }
}

static void __thiscall ball_update(void* thisptr, void* ball) {
    if (!g_enabled || !ball) return;
    apply_water_physics((DWORD)ball);
}

static void __thiscall scene_end(void*) {
    memset(g_states, 0, sizeof(g_states));
}

static void __thiscall level_start(void*) {
    memset(g_states, 0, sizeof(g_states));
}

static void __thiscall render_apply(void*, void*, float*) {}
static void __thiscall cycle_option_change(void*, const char*, const char*) {}
static void __thiscall text_render(void*) {}
static void __thiscall ball_bump(void*, void*, void*) {}

/* 17-entry vtable (HB+ v2.0/v2.1) */
static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]  ~HamsterballAPI
    (void*)get_mod_name,         // [1]  GetModName
    (void*)get_author,           // [2]  GetAuthorName
    (void*)get_version,          // [3]  GetApiVersion
    (void*)get_contributors,     // [4]  GetContributors
    (void*)init_impl,            // [5]  Initialize
    (void*)ball_update,          // [6]  onBallUpdate
    (void*)render_apply,         // [7]  onRenderApply
    (void*)button_toggle,        // [8]  onButtonToggle
    (void*)slider_change,        // [9]  onSliderChange
    (void*)cycle_option_change,  // [10] onCycleOptionChange (v2.0+)
    (void*)game_update,          // [11] onGameUpdate
    (void*)event_collide,        // [12] onEventPlaneCollide
    (void*)text_render,          // [13] onTextRenderLoop
    (void*)ball_bump,            // [14] onBallBump
    (void*)scene_end,            // [15] onSceneEnd
    (void*)level_start,          // [16] onLevelStart
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = operator new(8);
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}